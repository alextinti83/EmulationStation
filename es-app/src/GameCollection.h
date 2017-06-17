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
	public:
		Game() : m_filedata(nullptr) { }
		Game(const FileData& fd) : m_filedata(&fd) { }
		const FileData& GetFiledata() const { assert(m_filedata); return *m_filedata; }
		bool IsValid() const { return m_filedata != nullptr; }
	private:
		const FileData* m_filedata;
	};

public:
	enum class Tag { None, Highlighted, Hidden };

	GameCollection(const std::string& name, const std::string& folderPath);
	void Rename(const std::string& name);
	void EraseFile();
	
	bool Deserialize();
	bool Serialize();
	
	bool HasGame(const FileData& filedata) const;
	void AddGame(const FileData& filedata);
	void RemoveGame(const FileData& filedata);
	const std::string& GetName() const;

	bool HasTag(Tag tag) const { return m_tag == tag; }
	void SetTag(Tag tag) { m_tag = tag; }

	// since we serialize/deserialize only key
	// we need to map filedatas to their respective key
	void ReplacePlaceholder(const FileData& filedata); 
public:
	static std::string GetTagName(Tag tag);
	static const std::vector<Tag> GetTags();


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
	Tag m_tag;

private:
	static const std::map<GameCollection::Tag, std::string> k_tagsNames;
	static const std::map<std::string, GameCollection::Tag> k_namesTags;

};