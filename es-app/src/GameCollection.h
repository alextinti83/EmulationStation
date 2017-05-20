#pragma once

#include <string>
#include <map>
#include <assert.h>
#include "boost/filesystem/string_file.hpp"


class FileData;
class GameCollection
{
private:
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

	using GamesMap = std::map<std::string, Game>;
public:
	class View
	{
	public:
		class iterator {
		public:
			iterator(
				const GamesMap::const_iterator& it,
				const GamesMap::const_iterator& eIt);
			iterator operator++();
			bool operator!=(const iterator & other);
			const FileData& operator*() const;
		private:
			GamesMap::const_iterator mapIt;
			GamesMap::const_iterator endIt;

		};

		View(const GameCollection& gameCollection);
		iterator begin() const;
		iterator end() const;

	private:
		const GameCollection& m_gameCollection;
	};

public:
	GameCollection(const std::string& name);

	void Deserialize(const boost::filesystem::path& folderPath);
	void Serialize(const boost::filesystem::path& folderPath);
	bool HasGame(const FileData& filedata) const;
	void AddGame(const FileData& filedata);
	void RemoveGame(const FileData& filedata);

	// since we deserialize only keys
	// we need to map filedatas to their 
	// respective keys once filedatas are created
	void ReplacePlaceholder(const FileData& filedata); 

	const View& GetView() const { return m_view;  }
private:
	std::string getFilePath(const boost::filesystem::path& folderPath) const;


	std::string GetKey(const FileData& filedata) const;
private:
	const std::string m_name;

	GamesMap mGamesMap;
	View m_view;
};

