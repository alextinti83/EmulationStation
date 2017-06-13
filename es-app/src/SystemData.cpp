#include "SystemData.h"
#include "Gamelist.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <stdlib.h>
#include <SDL_joystick.h>
#include "Renderer.h"
#include "AudioManager.h"
#include "VolumeControl.h"
#include "Log.h"
#include "InputManager.h"
#include <iostream>
#include "Settings.h"
#include "FileSorts.h"
#include "GameCollection.h"
#include "EmulationStation.h"

std::vector<SystemData*> SystemData::sSystemVector;

namespace fs = boost::filesystem;

SystemData::SystemData(
	const std::string& name,
	const std::string& fullName,
	const std::string& startPath,
	const std::vector<std::string>& extensions,
	const std::string& command,
	const std::vector<PlatformIds::PlatformId>& platformIds,
	const std::string& themeFolder,
	const bool enabled)
	//: mFavorites()
	: m_enabled(enabled)
	, mGameCollectionsPath(".emulationstation/game_collections")
	, mCurrentCollectionKey("favorites")
{
	mName = name;
	mFullName = fullName;
	mStartPath = startPath;

	//expand home symbol if the startpath contains ~
	if (mStartPath[ 0 ] == '~')
	{
		mStartPath.erase(0, 1);
		mStartPath.insert(0, getHomePath());
	}

	mSearchExtensions = extensions;
	mLaunchCommand = command;
	mPlatformIds = platformIds;
	mThemeFolder = themeFolder;

	mFilterIndex = new FileFilterIndex();

	mRootFolder = new FileData(FOLDER, mStartPath, this);
	mRootFolder->metadata.set("name", mFullName);

	if (m_enabled)
	{
		if (!Settings::getInstance()->getBool("ParseGamelistOnly"))
		{
			populateFolder(mRootFolder);
		}

	
		LoadGameCollections();

		if (!Settings::getInstance()->getBool("IgnoreGamelist"))
		{
			parseGamelist(this);
		}

		mRootFolder->sort(FileSorts::SortTypes.at(0));

		loadTheme();
	}
}

SystemData::~SystemData()
{
	if (m_enabled)
	{
		SaveGameCollections();

		//save changed game data back to xml
		if (!Settings::getInstance()->getBool("IgnoreGamelist") && Settings::getInstance()->getBool("SaveGamelistsOnExit"))
		{
			writeGamelistToFile(this);
		}
	}

	delete mRootFolder;
	delete mFilterIndex;
}

void SystemData::ImportLegacyFavoriteGameCollection()
{
	const std::string favname = "favorites.xml";
	const boost::filesystem::path legacyFavPath = mRootFolder->getPath() / favname;
	if (boost::filesystem::exists(legacyFavPath))
	{
		const boost::filesystem::path newPath = mRootFolder->getPath() / mGameCollectionsPath / favname;
		if (!boost::filesystem::exists(newPath))
		{
			boost::filesystem::rename(legacyFavPath, newPath);
		}
	}
}

void SystemData::LoadGameCollections()
{
	//mFavorites.reset(new GameCollection("favorites"));
	//mFavorites->Deserialize(mRootFolder->getPath());

	ImportLegacyFavoriteGameCollection();

	using GameCollectionIt = std::map<std::string, GameCollection>::iterator;

	boost::filesystem::path absCollectionsPath(mRootFolder->getPath() / mGameCollectionsPath);
	if (boost::filesystem::exists(absCollectionsPath))
	{
		using fsIt = boost::filesystem::recursive_directory_iterator;
		fsIt end;
		for (fsIt i(absCollectionsPath); i != end; ++i)
		{
			const boost::filesystem::path cp = ( *i );
			if (!boost::filesystem::is_directory(cp))
			{
				const std::string filename = cp.filename().generic_string();
				const std::string key = cp.stem().generic_string();
				LOG(LogInfo) << "Loading GameCollection: " << filename;

				mGameCollections.emplace(key, key);
				GameCollectionIt collectionIt = mGameCollections.find(key);
				if (collectionIt != mGameCollections.end())
				{
					if (!collectionIt->second.Deserialize(absCollectionsPath.generic_string()))
					{
						mGameCollections.erase(collectionIt);
					}
				}
			}
		}
	}
}

bool SystemData::SaveGameCollections()
{
	//if (mFavorites)
	//{
	//	mFavorites->Serialize(mRootFolder->getPath());
	//}

	boost::filesystem::path absCollectionsPath(mRootFolder->getPath() / mGameCollectionsPath);
	if (!boost::filesystem::exists(absCollectionsPath))
	{
		boost::system::error_code returnedError;
		boost::filesystem::create_directories(absCollectionsPath, returnedError);
		if (returnedError)
		{
			return false;
		}
	}

	using GameCollectionMapValueType = std::map<std::string, GameCollection>::value_type;
	for (GameCollectionMapValueType& pair : mGameCollections)
	{
		pair.second.Serialize(absCollectionsPath.generic_string());	
	}
	return true;
}

const GameCollection* SystemData::GetCurrentGameCollection() const
{
	return GetGameCollection(mCurrentCollectionKey);
}

GameCollection* SystemData::GetCurrentGameCollection()
{
	const SystemData& const_this = static_cast< const SystemData& >( *this );
	return const_cast< GameCollection* >( const_this.GetCurrentGameCollection());
}

const SystemData::GameCollections& SystemData::GetGameCollections() const
{
	return mGameCollections;
}

bool SystemData::NewGameCollection(const std::string& key)
{
	if (GetGameCollection(key))
	{
		return false;
	}
	mGameCollections.emplace(key, key);
	return true;
}

bool SystemData::SetCurrentGameCollection(const std::string& key)
{
	if (GetGameCollection(key))
	{
		mCurrentCollectionKey = key;
		return true;
	}
	return false;
}

GameCollection* SystemData::GetGameCollection(const std::string& key)
{
	const SystemData& const_this = static_cast< const SystemData& >( *this );
	return const_cast< GameCollection* >( const_this.GetGameCollection(key) );
}

const GameCollection* SystemData::GetGameCollection(const std::string& key) const
{
	using GameCollectionIt = std::map<std::string, GameCollection>::const_iterator;
	GameCollectionIt collectionIt = mGameCollections.find(key);
	if (collectionIt != mGameCollections.end())
	{
		return &collectionIt->second;
	}
	return nullptr;
}

std::string strreplace(std::string str, const std::string& replace, const std::string& with)
{
	size_t pos;
	while (( pos = str.find(replace) ) != std::string::npos)
		str = str.replace(pos, replace.length(), with.c_str(), with.length());

	return str;
}

// plaform-specific escape path function
// on windows: just puts the path in quotes
// everything else: assume bash and escape special characters with backslashes
std::string escapePath(const boost::filesystem::path& path)
{
#ifdef WIN32
	// windows escapes stuff by just putting everything in quotes
	return '"' + fs::path(path).make_preferred().string() + '"';
#else
	// a quick and dirty way to insert a backslash before most characters that would mess up a bash path
	std::string pathStr = path.string();

	const char* invalidChars = " '\"\\!$^&*(){}[]?;<>";
	for (unsigned int i = 0; i < pathStr.length(); i++)
	{
		char c;
		unsigned int charNum = 0;
		do
		{
			c = invalidChars[ charNum ];
			if (pathStr[ i ] == c)
			{
				pathStr.insert(i, "\\");
				i++;
				break;
			}
			charNum++;
		}
		while (c != '\0');
	}

	return pathStr;
#endif
}

void SystemData::launchGame(Window* window, FileData* game)
{
	LOG(LogInfo) << "Attempting to launch game...";

	AudioManager::getInstance()->deinit();
	VolumeControl::getInstance()->deinit();
	window->deinit();

	std::string command = mLaunchCommand;

	const std::string rom = escapePath(game->getPath());
	const std::string basename = game->getPath().stem().string();
	const std::string rom_raw = fs::path(game->getPath()).make_preferred().string();

	command = strreplace(command, "%ROM%", rom);
	command = strreplace(command, "%BASENAME%", basename);
	command = strreplace(command, "%ROM_RAW%", rom_raw);

	LOG(LogInfo) << "	" << command;
	int exitCode = runSystemCommand(command);

	if (exitCode != 0)
	{
		LOG(LogWarning) << "...launch terminated with nonzero exit code " << exitCode << "!";
	}

	window->init();
	VolumeControl::getInstance()->init();
	AudioManager::getInstance()->init();
	window->normalizeNextUpdate();

	//update number of times the game has been launched
	int timesPlayed = game->metadata.getInt("playcount") + 1;
	game->metadata.set("playcount", std::to_string(static_cast< long long >( timesPlayed )));

	//update last played time
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	game->metadata.setTime("lastplayed", time);
}

void SystemData::populateFolder(FileData* folder)
{
	const fs::path& folderPath = folder->getPath();
	if (!fs::is_directory(folderPath))
	{
		LOG(LogWarning) << "Error - folder with path \"" << folderPath << "\" is not a directory!";
		return;
	}

	const std::string folderStr = folderPath.generic_string();

	//make sure that this isn't a symlink to a thing we already have
	if (fs::is_symlink(folderPath))
	{
		//if this symlink resolves to somewhere that's at the beginning of our path, it's gonna recurse
		if (folderStr.find(fs::canonical(folderPath).generic_string()) == 0)
		{
			LOG(LogWarning) << "Skipping infinitely recursive symlink \"" << folderPath << "\"";
			return;
		}
	}

	fs::path filePath;
	std::string extension;
	bool isGame;
	for (fs::directory_iterator end, dir(folderPath); dir != end; ++dir)
	{
		filePath = ( *dir ).path();

		if (filePath.stem().empty())
			continue;

		//this is a little complicated because we allow a list of extensions to be defined (delimited with a space)
		//we first get the extension of the file itself:
		extension = filePath.extension().string();

		//fyi, folders *can* also match the extension and be added as games - this is mostly just to support higan
		//see issue #75: https://github.com/Aloshi/EmulationStation/issues/75

		isGame = false;
		if (std::find(mSearchExtensions.begin(), mSearchExtensions.end(), extension) != mSearchExtensions.end())
		{
			FileData* newGame = new FileData(GAME, filePath.generic_string(), this);
			folder->addChild(newGame);
			isGame = true;
		}

		//add directories that also do not match an extension as folders
		if (!isGame && fs::is_directory(filePath))
		{
			FileData* newFolder = new FileData(FOLDER, filePath.generic_string(), this);
			populateFolder(newFolder);

			//ignore folders that do not contain games
			if (newFolder->getChildrenByFilename().size() == 0)
				delete newFolder;
			else
				folder->addChild(newFolder);
		}
	}
}

std::vector<std::string> readList(const std::string& str, const char* delims = " \t\r\n,")
{
	std::vector<std::string> ret;

	size_t prevOff = str.find_first_not_of(delims, 0);
	size_t off = str.find_first_of(delims, prevOff);
	while (off != std::string::npos || prevOff != std::string::npos)
	{
		ret.push_back(str.substr(prevOff, off - prevOff));

		prevOff = str.find_first_not_of(delims, off);
		off = str.find_first_of(delims, prevOff);
	}

	return ret;
}

static std::string k_enabledNodeName = "enabled";


bool SystemData::SaveConfig()
{

	std::string path = getConfigPath(false);
	LOG(LogInfo) << "Loading system config file " << path << "...";

	if (!fs::exists(path))
	{
		LOG(LogError) << "es_systems.cfg file does not exist!";
		writeExampleConfig(getConfigPath(true));
		return false;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str(), pugi::parse_default | pugi::parse_comments);

	if (!res)
	{
		LOG(LogError) << "Could not parse es_systems.cfg file!";
		LOG(LogError) << res.description();
		return false;
	}

	pugi::xml_node systemList = doc.child("systemList");

	if (!systemList)
	{
		LOG(LogError) << "es_systems.cfg is missing the <systemList> tag!";
		return false;
	}
	bool needsUpdated = false;

	for (pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		const std::string name = system.child("name").text().get();
		const bool enabled = IsSystemEnabled(name);
		pugi::xml_node enabledNode = system.child(k_enabledNodeName.c_str());
		if (enabledNode)
		{
			if (enabled)
			{
				system.remove_child(k_enabledNodeName.c_str());
				needsUpdated = true;
				LOG(LogInfo) << "System " << name << " needs update" << std::endl;
			}
			else
			{
				pugi::xml_node descr = enabledNode.first_child();
				if (descr && descr.value() == "false")
				{
					enabledNode.set_value("false");
					needsUpdated = true;
					LOG(LogInfo) << "System " << name << " needs update" << std::endl;
				}
			}
		}
		else if (enabled == false)
		{
			pugi::xml_node descr = system.prepend_child(k_enabledNodeName.c_str());
			descr.append_child(pugi::node_pcdata).set_value("false");
			needsUpdated = true;
			LOG(LogInfo) << "System " << name << " needs update" << std::endl;
		}
		
	}

	if (needsUpdated)
	{
		static const std::string k_signaturePrefix = "Processed by ES ";
		static const std::string k_signature = k_signaturePrefix + std::string(PROGRAM_VERSION_STRING);
		pugi::xml_node firstChild = doc.first_child();
		if (firstChild && firstChild.type() == pugi::xml_node_type::node_comment)
		{
			if (std::string(firstChild.value()).find(k_signaturePrefix) == std::string::npos)
			{
				using namespace boost::posix_time;
				using namespace boost::gregorian;
				const auto date = second_clock::local_time().date();
				const auto time = second_clock::local_time().time_of_day();
				const std::string datePrefix = to_simple_string(date);
				const std::string timePrefix = to_simple_string(time);
				const std::string signature = k_signature + " on " + datePrefix + " at " + timePrefix.c_str();
				doc.prepend_child(pugi::node_comment).set_value(signature.c_str());
			}
		}

		const std::string writeConfigPath = getConfigPath(true);
		boost::filesystem::path resolvedPath = boost::filesystem::canonical(writeConfigPath);

		LOG(LogInfo) << "Updating " << resolvedPath << std::endl;
		if (doc.save_file(resolvedPath.generic_string().c_str()))
		{
			LOG(LogError) << "Error saving " << resolvedPath << std::endl;
			return false;
		}
	}
	else
	{
		LOG(LogInfo) << "Not need to update " << path << std::endl;
	}
	return true;
}


//creates systems from information located in a config file
bool SystemData::loadConfig()
{
	deleteSystems();

	std::string path = getConfigPath(false);
	LOG(LogInfo) << "Loading system config file " << path << "...";

	if (!fs::exists(path))
	{
		LOG(LogError) << "es_systems.cfg file does not exist!";
		writeExampleConfig(getConfigPath(true));
		return false;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result res = doc.load_file(path.c_str());

	if (!res)
	{
		LOG(LogError) << "Could not parse es_systems.cfg file!";
		LOG(LogError) << res.description();
		return false;
	}

	//actually read the file
	pugi::xml_node systemList = doc.child("systemList");

	if (!systemList)
	{
		LOG(LogError) << "es_systems.cfg is missing the <systemList> tag!";
		return false;
	}

	for (pugi::xml_node system = systemList.child("system"); system; system = system.next_sibling("system"))
	{
		std::string name, fullname, path, cmd, themeFolder;
		PlatformIds::PlatformId platformId = PlatformIds::PLATFORM_UNKNOWN;

		name = system.child("name").text().get();
		fullname = system.child("fullname").text().get();
		path = system.child("path").text().get();
		bool enabled = true;
		pugi::xml_node enabledNode = system.child(k_enabledNodeName.c_str());
		if (enabledNode)
		{
			const auto hiddenText = enabledNode.text().get();
			enabled = std::string(hiddenText) == std::string("true");
		}


		// convert extensions list from a string into a vector of strings
		std::vector<std::string> extensions = readList(system.child("extension").text().get());

		cmd = system.child("command").text().get();

		// platform id list
		const char* platformList = system.child("platform").text().get();
		std::vector<std::string> platformStrs = readList(platformList);
		std::vector<PlatformIds::PlatformId> platformIds;
		for (auto it = platformStrs.begin(); it != platformStrs.end(); it++)
		{
			const char* str = it->c_str();
			PlatformIds::PlatformId platformId = PlatformIds::getPlatformId(str);

			if (platformId == PlatformIds::PLATFORM_IGNORE)
			{
				// when platform is ignore, do not allow other platforms
				platformIds.clear();
				platformIds.push_back(platformId);
				break;
			}

			// if there appears to be an actual platform ID supplied but it didn't match the list, warn
			if (str != NULL && str[ 0 ] != '\0' && platformId == PlatformIds::PLATFORM_UNKNOWN)
				LOG(LogWarning) << "  Unknown platform for system \"" << name << "\" (platform \"" << str << "\" from list \"" << platformList << "\")";
			else if (platformId != PlatformIds::PLATFORM_UNKNOWN)
				platformIds.push_back(platformId);
		}

		// theme folder
		themeFolder = system.child("theme").text().as_string(name.c_str());

		//validate
		if (name.empty() || path.empty() || extensions.empty() || cmd.empty())
		{
			LOG(LogError) << "System \"" << name << "\" is missing name, path, extension, or command!";
			continue;
		}

		//convert path to generic directory seperators
		boost::filesystem::path genericPath(path);
		path = genericPath.generic_string();

		SystemData* newSys = new SystemData(name, fullname, path, extensions, cmd, platformIds, themeFolder, enabled);
		if (enabled && newSys->getRootFolder()->getChildrenByFilename().size() == 0 )
		{
			LOG(LogWarning) << "System \"" << name << "\" has no games! Ignoring it.";
			delete newSys;
		}
		else
		{
			sSystemVector.push_back(newSys);
		}
	}

	return true;
}

bool SystemData::IsSystemEnabled(const std::string& name)
{
	for (SystemData* system : SystemData::GetAllSystems())
	{
		if (name == system->getName())
		{
			return system->IsEnabled();
		}
	}
	return true; // this should not happen
}
void SystemData::writeExampleConfig(const std::string& path)
{
	std::ofstream file(path.c_str());

	file << "<!-- This is the EmulationStation Systems configuration file.\n"
		"All systems must be contained within the <systemList> tag.-->\n"
		"\n"
		"<systemList>\n"
		"	<!-- Here's an example system to get you started. -->\n"
		"	<system>\n"
		"\n"
		"		<!-- A short name, used internally. Traditionally lower-case. -->\n"
		"		<name>nes</name>\n"
		"\n"
		"		<!-- A \"pretty\" name, displayed in menus and such. -->\n"
		"		<fullname>Nintendo Entertainment System</fullname>\n"
		"\n"
		"		<!-- The path to start searching for ROMs in. '~' will be expanded to $HOME on Linux or %HOMEPATH% on Windows. -->\n"
		"		<path>~/roms/nes</path>\n"
		"\n"
		"		<!-- A list of extensions to search for, delimited by any of the whitespace characters (\", \\r\\n\\t\").\n"
		"		You MUST include the period at the start of the extension! It's also case sensitive. -->\n"
		"		<extension>.nes .NES</extension>\n"
		"\n"
		"		<!-- The shell command executed when a game is selected. A few special tags are replaced if found in a command:\n"
		"		%ROM% is replaced by a bash-special-character-escaped absolute path to the ROM.\n"
		"		%BASENAME% is replaced by the \"base\" name of the ROM.  For example, \"/foo/bar.rom\" would have a basename of \"bar\". Useful for MAME.\n"
		"		%ROM_RAW% is the raw, unescaped path to the ROM. -->\n"
		"		<command>retroarch -L ~/cores/libretro-fceumm.so %ROM%</command>\n"
		"\n"
		"		<!-- The platform to use when scraping. You can see the full list of accepted platforms in src/PlatformIds.cpp.\n"
		"		It's case sensitive, but everything is lowercase. This tag is optional.\n"
		"		You can use multiple platforms too, delimited with any of the whitespace characters (\", \\r\\n\\t\"), eg: \"genesis, megadrive\" -->\n"
		"		<platform>nes</platform>\n"
		"\n"
		"		<!-- The theme to load from the current theme set.  See THEMES.md for more information.\n"
		"		This tag is optional. If not set, it will default to the value of <name>. -->\n"
		"		<theme>nes</theme>\n"
		"	</system>\n"
		"</systemList>\n";

	file.close();

	LOG(LogError) << "Example config written!  Go read it at \"" << path << "\"!";
}

void SystemData::deleteSystems()
{
	for (unsigned int i = 0; i < sSystemVector.size(); i++)
	{
		delete sSystemVector.at(i);
	}
	sSystemVector.clear();
}

std::string SystemData::getConfigPath(bool forWrite)
{
	fs::path path = getHomePath() + "/.emulationstation/es_systems.cfg";
	if (forWrite || fs::exists(path))
		return path.generic_string();

	return "/etc/emulationstation/es_systems.cfg";
}
boost::filesystem::path SystemData::getRetroArchConfigImportFolder() const
{
	return mRootFolder->getPath() / "retroarch.cfg/";
}

boost::filesystem::path SystemData::getRetroArchSystemConfigFilepath() const
{
	return "/opt/retropie/configs/" + mName + "/retroarch.cfg";
}

std::string SystemData::getGamelistPath(bool forWrite) const
{
	fs::path filePath;

	filePath = mRootFolder->getPath() / "gamelist.xml";
	if (fs::exists(filePath))
		return filePath.generic_string();

	filePath = getHomePath() + "/.emulationstation/gamelists/" + mName + "/gamelist.xml";
	if (forWrite) // make sure the directory exists if we're going to write to it, or crashes will happen
		fs::create_directories(filePath.parent_path());
	if (forWrite || fs::exists(filePath))
		return filePath.generic_string();

	return "/etc/emulationstation/gamelists/" + mName + "/gamelist.xml";
}

std::string SystemData::getThemePath() const
{
	// where we check for themes, in order:
	// 1. [SYSTEM_PATH]/theme.xml
	// 2. currently selected theme set

	// first, check game folder
	fs::path localThemePath = mRootFolder->getPath() / "theme.xml";
	if (fs::exists(localThemePath))
		return localThemePath.generic_string();

	// not in game folder, try theme sets
	return ThemeData::getThemeFromCurrentSet(mThemeFolder).generic_string();
}

bool SystemData::hasGamelist() const
{
	return ( fs::exists(getGamelistPath(false)) );
}

unsigned int SystemData::getGameCount() const
{
	return mRootFolder->getFilesRecursive(GAME).size();
}

unsigned int SystemData::getDisplayedGameCount() const
{
	return mRootFolder->getFilesRecursive(GAME, true).size();
}


SystemData* SystemData::getPrevEnabled() const
{
	return findNextIfEnabled(sSystemVector.rbegin(), sSystemVector.rend());
}

SystemData* SystemData::getNextEnabled() const
{
	return findNextIfEnabled(sSystemVector.begin(), sSystemVector.end());
}

void SystemData::loadTheme()
{
	mTheme = std::make_shared<ThemeData>();

	std::string path = getThemePath();

	if (!fs::exists(path)) // no theme available for this platform
		return;

	try
	{
		mTheme->loadFile(path);
	}
	catch (ThemeException& e)
	{
		LOG(LogError) << e.what();
		mTheme = std::make_shared<ThemeData>(); // reset to empty
	}
}



bool SystemData::isInCurrentGameCollection(const FileData& filedata) const
{
	const auto collection = GetCurrentGameCollection();
	return collection && collection->HasGame(filedata);
	//return mFavorites->HasGame(filedata);
}

void SystemData::removeFromCurrentGameCollection(const FileData& filedata)
{
	auto collection = GetCurrentGameCollection();
	if (collection) { collection->RemoveGame(filedata); }
	//mFavorites->RemoveGame(filedata);
}

void SystemData::addToCurrentGameCollection(const FileData& filedata)
{
	auto collection = GetCurrentGameCollection();
	if (collection) { collection->AddGame(filedata); }
	//mFavorites->AddGame(filedata);
}


void SystemData::replaceGameCollectionPlacholder(const FileData& filedata)
{
	using GameCollectionMapValueType = std::map<std::string, GameCollection>::value_type;
	for (GameCollectionMapValueType& pair : mGameCollections)
	{
		pair.second.ReplacePlaceholder(filedata);
	}
	//mFavorites->ReplacePlaceholder(filedata);
}

void SystemData::SetEnabled(const bool enabled)
{
	m_enabled = enabled;
}

bool SystemData::IsEnabled() const
{
	return m_enabled;
}

std::vector<SystemData*> SystemData::GetSystems()
{
	std::vector<SystemData*> systems;

	for (auto& system : sSystemVector)
	{
		if (system->IsEnabled())
		{
			systems.push_back(system);
		}
	}
	return systems;
}

std::vector<SystemData*> SystemData::GetAllSystems()
{
	std::vector<SystemData*> systems;

	for (auto& system : sSystemVector)
	{
		systems.push_back(system);
	}
	return systems;
}
