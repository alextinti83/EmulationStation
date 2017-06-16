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

public:
	GameCollections(const FileData& rootFolder);
	~GameCollections();
	//init
	void ReplaceGameCollectionPlacholder(const FileData& filedata);
	void ReplaceAllPlacholdersForGameCollection(const std::string& gameCollectionKey);

	
	//getters
	const GameCollection* GetGameCollection(const std::string& key) const;
	const GameCollection* GetCurrentGameCollection() const;
	const GameCollectionMap& GetGameCollections() const;
	GameCollection* GetGameCollection(const std::string& key);
	GameCollection* GetCurrentGameCollection();

	bool IsInCurrentGameCollection(const FileData& filedata) const;
	
	//actions
	bool NewGameCollection(const std::string& key);
	bool DeleteGameCollection(const std::string& key);
	bool RenameGameCollection(const std::string& key, const std::string& newKey);

	bool SetCurrentGameCollection(const std::string& key);

	void RemoveFromCurrentGameCollection(const FileData& filedata);
	void AddToCurrentGameCollection(const FileData& filedata);

	

	//serialization
	void LoadGameCollections();
	bool SaveGameCollections();

private:
	void ImportLegacyFavoriteGameCollection();

private:
	GameCollectionMap mGameCollections;
	std::string mGameCollectionsPath;
	std::string mCurrentCollectionKey;

	const FileData& mRootFolder;
};