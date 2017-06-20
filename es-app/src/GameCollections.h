#pragma once

#include <string>
#include <map>
#include <assert.h>
#include "boost/filesystem.hpp"

#include "GameCollection.h"

class FileData;

class GameCollections
{
public:
	using GameCollectionMap = std::map<std::string, GameCollection>;
	using GameCollectionList = std::vector<const GameCollection*>;
public:
	GameCollections(const FileData& rootFolder);
	~GameCollections();
	//init
	void ReplaceGameCollectionPlacholder(const FileData& filedata);
	void ReplaceAllPlacholdersForGameCollection(const std::string& gameCollectionKey);

	
	//getters
	const GameCollection* GetGameCollection(const std::string& key) const;
	const GameCollection* GetActiveGameCollection() const;
	const GameCollectionMap& GetGameCollectionMap() const;

	GameCollection* GetGameCollection(const std::string& key);
	GameCollection* GetActiveGameCollection();

	bool IsInActivetGameCollection(const FileData& filedata) const;
	bool HasTag(const FileData& filedata, GameCollection::Tag tag) const;

	//actions
	bool NewGameCollection(const std::string& key);
	bool DeleteGameCollection(const std::string& key);
	bool RenameGameCollection(const std::string& key, const std::string& newKey);
	bool DuplicateGameCollection(const std::string& key, const std::string& duplicateKey);


	bool SetActiveGameCollection(const std::string& key);

	void RemoveFromActiveGameCollection(const FileData& filedata);
	void AddToActiveGameCollection(const FileData& filedata);

	

	//serialization
	void LoadGameCollections();
	bool SaveGameCollections();
	bool SaveSettings();
	bool LoadSettings();


private:
	void ImportLegacyFavoriteGameCollection();

private:
	GameCollectionMap mGameCollections;
	std::string mGameCollectionsPath;
	std::string mActiveCollectionKey;

	const FileData& mRootFolder;

	static boost::filesystem::path k_emulationStationFolder;
	static boost::filesystem::path k_gameCollectionsFolder;

};