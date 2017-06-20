#include "GameCollections.h"
#include "pugixml/src/pugixml.hpp"
#include "Log.h"

#include "FileData.h"

boost::filesystem::path GameCollections::k_emulationStationFolder(".emulationstation");
boost::filesystem::path GameCollections::k_gameCollectionsFolder("game_collections");


static std::string k_autoCreatedCollection = "Favorites";

GameCollections::GameCollections(const FileData& rootFolder)
	: mRootFolder(rootFolder)
	, mGameCollectionsPath(
		(k_emulationStationFolder / k_gameCollectionsFolder).generic_string()
	)
	, mActiveCollectionKey("")
{
}

GameCollections::~GameCollections()
{
}

bool CreateDir(boost::filesystem::path path)
{
	if (!boost::filesystem::exists(path))
	{
		boost::system::error_code returnedError;
		boost::filesystem::create_directories(path, returnedError);
		if (returnedError)
		{
			return false;
		}
	}
	return true;
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
	boost::filesystem::path absCollectionsPath(mRootFolder.getPath() / mGameCollectionsPath);
	CreateDir(absCollectionsPath);

	LoadSettings();
	ImportLegacyFavoriteGameCollection();

	using GameCollectionIt = std::map<std::string, GameCollection>::iterator;

	bool activeFound = false;
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
						if (collectionIt->first == mActiveCollectionKey)
						{
							activeFound = true;
						}
					}
				}
				else
				{
					LOG(LogError) << "Duplicated name for GameCollection: " << filename << " [Not loaded]";
				}
			}
		}
		if (mGameCollections.empty())
		{
			GameCollection gameCollection(k_autoCreatedCollection, absCollectionsPath.generic_string());
			auto result = mGameCollections.emplace(std::make_pair(k_autoCreatedCollection, std::move(gameCollection)));
		}
		if (!activeFound)
		{
			mActiveCollectionKey = mGameCollections.begin()->first;
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
	pugi::xml_attribute attr = root.append_attribute("active");
	attr.set_value(mActiveCollectionKey.c_str());

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
			mActiveCollectionKey = root.attribute("active").as_string();
		}
		else
		{
			LOG(LogError) << "Could parsing game collection list: \"" << xmlPath << "\"!";
			return false;
		}
	}
	return true;
}

bool GameCollections::SaveGameCollections()
{
	boost::filesystem::path absCollectionsPath(mRootFolder.getPath() / mGameCollectionsPath);
	CreateDir(absCollectionsPath);
	
	SaveSettings();

	using GameCollectionMapValueType = std::map<std::string, GameCollection>::value_type;
	for (GameCollectionMapValueType& pair : mGameCollections)
	{
		pair.second.Serialize();
	}
	return true;
}

const GameCollection* GameCollections::GetActiveGameCollection() const
{
	return GetGameCollection(mActiveCollectionKey);
}

GameCollection* GameCollections::GetActiveGameCollection()
{
	const GameCollections& const_this = static_cast< const GameCollections& >( *this );
	return const_cast< GameCollection* >( const_this.GetActiveGameCollection() );
}

const GameCollections::GameCollectionMap& GameCollections::GetGameCollectionMap() const
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
	if (mActiveCollectionKey.size() <= 1)
	{
		return false;
	}
	GameCollection* collection = GetGameCollection(key);
	if (collection)
	{
		collection->EraseFile();
		using GameCollectionIt = std::map<std::string, GameCollection>::const_iterator;
		mGameCollections.erase(key);
		if (mActiveCollectionKey == key)
		{
			if (mGameCollections.size() > 0)
			{
				mActiveCollectionKey = mGameCollections.begin()->first;
			}
			else
			{
				mActiveCollectionKey = "";
			}
		}
		return true;
	}
	return false;
}

bool GameCollections::RenameGameCollection(const std::string& key, const std::string& newKey)
{
	GameCollection* collection = GetGameCollection(key);
	if (collection)
	{
		if (key == mActiveCollectionKey)
		{
			mActiveCollectionKey = newKey;
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

bool GameCollections::SetActiveGameCollection(const std::string& key)
{
	if (GetGameCollection(key))
	{
		mActiveCollectionKey = key;
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

bool GameCollections::IsInActivetGameCollection(const FileData& filedata) const
{
	const auto collection = GetActiveGameCollection();
	return collection && collection->HasGame(filedata);
}

bool GameCollections::HasTag(const FileData& filedata, GameCollection::Tag tag) const
{
	for (GameCollectionMap::value_type kv : mGameCollections)
	{
		if (kv.second.HasTag(tag))
		{
			if (kv.second.HasGame(filedata))
			{
				return true;
			}
		}
	}
	return false;
}

void GameCollections::RemoveFromActiveGameCollection(const FileData& filedata)
{
	auto collection = GetActiveGameCollection();
	if (collection) { collection->RemoveGame(filedata); }
}

void GameCollections::AddToActiveGameCollection(const FileData& filedata)
{
	auto collection = GetActiveGameCollection();
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