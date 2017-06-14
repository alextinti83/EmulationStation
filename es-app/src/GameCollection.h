#pragma once

#include <string>
#include <map>
#include <assert.h>
#include "boost/filesystem/path.hpp"


class FileData;

class GameCollection
{
public:
	class Game
	{
	public:
		Game() : m_filedata(nullptr) { }
		Game(const FileData& fd) : m_filedata(&fd) { }
		const FileData& GetFiledata() const { assert(m_filedata); return *m_filedata; }
		bool IsValid() const { return m_filedata != nullptr; }
	private:
		const FileData* m_filedata;
	};

public:
	GameCollection(const std::string& name, const std::string& folderPath);
	void Rename(const std::string& name);
	void EraseFile();

	bool Deserialize();
	bool Serialize();
	
	bool HasGame(const FileData& filedata) const;
	void AddGame(const FileData& filedata);
	void RemoveGame(const FileData& filedata);
	const std::string& GetName() const;
	// since we serialize/deserialize only key
	// we need to map filedatas to their respective key
	void ReplacePlaceholder(const FileData& filedata); 
private:
	std::string GetFilePath(const boost::filesystem::path& folderPath) const;
	bool Deserialize(const boost::filesystem::path& folderPath);
	bool Serialize(const boost::filesystem::path& folderPath);

	std::string GetKey(const FileData& filedata) const;
private:
	std::string m_name;
	std::string m_folderPath;

	using GamesMap = std::map<std::string, Game>;
	GamesMap mGamesMap;

};