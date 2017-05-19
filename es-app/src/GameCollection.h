#pragma once

#include <string>
#include <map>
#include <assert.h>
#include "boost/filesystem/string_file.hpp"


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
	GameCollection(const std::string& name);

	void Deserialize(const boost::filesystem::path& folderPath);
	void Serialize(const boost::filesystem::path& folderPath);
	bool HasGame(const FileData& filedata) const;
	void AddGame(const FileData& filedata);
	void RemoveGame(const FileData& filedata);

	// since we serialize/deserialize only key
	// we need to map filedatas to their respective key
	void ReplacePlaceholder(const FileData& filedata); 
private:
	std::string getFilePath(const boost::filesystem::path& folderPath) const;


	std::string GetKey(const FileData& filedata) const;
private:
	const std::string m_name;

	using GamesMap = std::map<std::string, Game>;
	GamesMap mGamesMap;

};