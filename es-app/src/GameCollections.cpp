#include "GameCollections.h"
#include "pugixml/src/pugixml.hpp"
#include "Log.h"

#include "FileData.h"

boost::filesystem::path GameCollections::k_emulationStationFolder(".emulationstation");
boost::filesystem::path GameCollections::k_gameCollectionsFolder("game_collections");




GameCollections::GameCollections(const FileData& rootFolder)
	: mRootFolder(rootFolder)
	, mGameCollectionsPath(
		(k_emulationStationFolder / k_gameCollectionsFolder).generic_string()
	)
	, mCurrentCollectionKey("favorites")
{
}

GameCollections::~GameCollections()
{
}

void GameCollections::ImportLegacyFavoriteGameCollection()
{
	const std::string favname = "favorites.xml";
	const boost::filesystem::path legacyFavPath = mRootFolder.getPath() / favname;
	if (boost::filesystem::exists(legacyFavPath))
	{
		const boost::filesystem::path newPath = mRootFolder.getPath() / mGameCollectionsPath / favname;
		if (!boost::filesystem::exists(newPath))
		{
			boost::filesystem::rename(legacyFavPath, newPath);
		}
	}
}

void GameCollections::LoadGameCollections()
{
	LoadSettings();
	ImportLegacyFavoriteGameCollection();

	using GameCollectionIt = std::map<std::string, GameCollection>::iterator;

	bool currentFound = false;
	boost::filesystem::path absCollectionsPath(mRootFolder.getPath() / mGameCollectionsPath);
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
				GameCollection gameCollection(key, absCollectionsPath.generic_string());
				auto result = mGameCollections.emplace(std::make_pair(key, std::move(gameCollection)));
				GameCollectionIt collectionIt = result.first;
				if (collectionIt != mGameCollections.end() && result.second)
				{
					if (!collectionIt->second.Deserialize())
					{
						LOG(LogError) << "De-serialization failed for GameCollection: " << filename;
						mGameCollections.erase(collectionIt);
					}
					else
					{
						if (collectionIt->first == mCurrentCollectionKey)
						{
							currentFound = true;
						}
					}
				}
				else
				{
					LOG(LogError) << "Duplicated name for GameCollection: " << filename << " [Not loaded]";
				}
			}
		}
		if (!currentFound && mGameCollections.empty())
		{
			mCurrentCollectionKey = mGameCollections.begin()->first;
		}
	}
}
static const std::string k_gamecollectionsTag = "game_collections";
bool GameCollections::SaveSettings()
{
	const boost::filesystem::path settings(mRootFolder.getPath() / ( mGameCollectionsPath + ".xml" ));
	pugi::xml_document doc;
	pugi::xml_node root;
	const bool forWrite = false;
	std::string xmlPath = settings.generic_string();

	root = doc.append_child(k_gamecollectionsTag.c_str());
	pugi::xml_attribute attr = root.append_attribute("current");
	attr.set_value(mCurrentCollectionKey.c_str());

	if (!doc.save_file(xmlPath.c_str()))
	{
		LOG(LogError) << "Error saving \"" << xmlPath << "!";
		return false;
	}
	return true;
}

bool GameCollections::LoadSettings()
{
	const boost::filesystem::path settings(mRootFolder.getPath() / (mGameCollectionsPath + ".xml"));

	pugi::xml_document doc;
	pugi::xml_node root;
	const bool forWrite = false;
	std::string xmlPath = settings.generic_string();

	if (boost::filesystem::exists(xmlPath))
	{
		pugi::xml_parse_result result = doc.load_file(xmlPath.c_str());
		pugi::xml_node root = doc.child(k_gamecollectionsTag.c_str());
		if (root)
		{
			mCurrentCollectionKey = root.attribute("current").as_string();
		}
		else
		{
			LOG(LogError) << "Could parsing favorites list: \"" << xmlPath << "\"!";
			return false;
		}
	}
	return true;
}

bool GameCollections::SaveGameCollections()
{
	SaveSettings();

	boost::filesystem::path absCollectionsPath(mRootFolder.getPath() / mGameCollectionsPath);
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
		pair.second.Serialize();
	}
	return true;
}

const GameCollection* GameCollections::GetCurrentGameCollection() const
{
	return GetGameCollection(mCurrentCollectionKey);
}

GameCollection* GameCollections::GetCurrentGameCollection()
{
	const GameCollections& const_this = static_cast< const GameCollections& >( *this );
	return const_cast< GameCollection* >( const_this.GetCurrentGameCollection() );
}

const GameCollections::GameCollectionMap& GameCollections::GetGameCollections() const
{
	return mGameCollections;
}

bool GameCollections::NewGameCollection(const std::string& key)
{
	if (GetGameCollection(key))
	{
		return false;
	}
	boost::filesystem::path absCollectionsPath(mRootFolder.getPath() / mGameCollectionsPath);
	GameCollection gameCollection(key, absCollectionsPath.generic_string());
	auto result = mGameCollections.emplace(std::make_pair(key, std::move(gameCollection)));
	return result.second;
}

bool GameCollections::DeleteGameCollection(const std::string& key)
{
	if (mCurrentCollectionKey.size() <= 1)
	{
		return false;
	}
	GameCollection* collection = GetGameCollection(key);
	if (collection)
	{
		collection->EraseFile();
		using GameCollectionIt = std::map<std::string, GameCollection>::const_iterator;
		mGameCollections.erase(key);
		mCurrentCollectionKey = mGameCollections.begin()->first;
		return true;
	}
	return false;
}

bool GameCollections::RenameGameCollection(const std::string& key, const std::string& newKey)
{
	GameCollection* collection = GetGameCollection(key);
	if (collection)
	{
		if (key == mCurrentCollectionKey)
		{
			mCurrentCollectionKey = newKey;
		}
		collection->Rename(newKey);
		mGameCollections.emplace(newKey, *collection); //copy
		mGameCollections.erase(key);
	}
	return false;
}

bool GameCollections::DuplicateGameCollection(const std::string& key, const std::string& duplicateKey)
{
	GameCollection* collection = GetGameCollection(key);
	if (collection && !GetGameCollection(duplicateKey))
	{
		mGameCollections.emplace(duplicateKey, *collection); //copy
		GameCollection* duplicate = GetGameCollection(duplicateKey);
		if (duplicate)
		{
			duplicate->Rename(duplicateKey);
			return true;
		}
	}
	return false;
}

bool GameCollections::SetCurrentGameCollection(const std::string& key)
{
	if (GetGameCollection(key))
	{
		mCurrentCollectionKey = key;
		return true;
	}
	return false;
}

GameCollection* GameCollections::GetGameCollection(const std::string& key)
{
	const GameCollections& const_this = static_cast< const GameCollections& >( *this );
	return const_cast< GameCollection* >( const_this.GetGameCollection(key) );
}

const GameCollection* GameCollections::GetGameCollection(const std::string& key) const
{
	using GameCollectionIt = std::map<std::string, GameCollection>::const_iterator;
	GameCollectionIt collectionIt = mGameCollections.find(key);
	if (collectionIt != mGameCollections.end())
	{
		return &collectionIt->second;
	}
	return nullptr;
}

bool GameCollections::IsInCurrentGameCollection(const FileData& filedata) const
{
	const auto collection = GetCurrentGameCollection();
	return collection && collection->HasGame(filedata);
}

void GameCollections::RemoveFromCurrentGameCollection(const FileData& filedata)
{
	auto collection = GetCurrentGameCollection();
	if (collection) { collection->RemoveGame(filedata); }
}

void GameCollections::AddToCurrentGameCollection(const FileData& filedata)
{
	auto collection = GetCurrentGameCollection();
	if (collection) { collection->AddGame(filedata); }
}

void GameCollections::ReplaceGameCollectionPlacholder(const FileData& filedata)
{
	using GameCollectionMapValueType = std::map<std::string, GameCollection>::value_type;
	for (GameCollectionMapValueType& pair : mGameCollections)
	{
		pair.second.ReplacePlaceholder(filedata);
	}
}

void GameCollections::ReplaceAllPlacholdersForGameCollection(const std::string& gameCollectionKey)
{
	GameCollection* collection = GetGameCollection(gameCollectionKey);
	if (collection)
	{
		for (FileData* filedata : mRootFolder.getFilesRecursive(GAME))
		{
			collection->ReplacePlaceholder(*filedata);
		}
	}
}