#include "GameCollection.h"

#include "pugixml/src/pugixml.hpp"
#include "Log.h"

#include "FileData.h"



GameCollection::GameCollection(
	const std::string& name, 
	const std::string& folderPath)
	: m_name(name)
	, m_folderPath(folderPath)
{
	// nothing to do
}


void GameCollection::Rename(const std::string& name)
{
	boost::filesystem::path oldPath = GetFilePath(m_folderPath);
	if (!m_folderPath.empty() && boost::filesystem::exists(oldPath))
	{
		m_name = name;
		boost::filesystem::path newPath = GetFilePath(m_folderPath);
		boost::filesystem::rename(oldPath, newPath);
	}
	else
	{
		m_name = name;
	}
}

std::string GameCollection::GetFilePath(const boost::filesystem::path& folderPath) const
{
	return ( folderPath / (m_name + ".xml") ).generic_string();
}



std::string GameCollection::GetKey(const FileData& filedata) const
{
	using MapT = std::map < std::string, std::string>;
	const MapT& metadata = filedata.metadata.GetMetadataMap();
	MapT::const_iterator pathIt = metadata.find("path");
	if (pathIt != metadata.cend())
	{
		return pathIt->second;
	}
	return filedata.getName();
}

bool GameCollection::HasGame(const FileData& filedata) const
{
	auto it = mGamesMap.find(GetKey(filedata));
	return it != mGamesMap.end() && it->second.IsValid();
}

void GameCollection::RemoveGame(const FileData& filedata)
{
	auto it = mGamesMap.find(GetKey(filedata));
	if ( it != mGamesMap.end() )
	{
		mGamesMap.erase(it);
	}
	else
	{
		LOG(LogWarning) << m_name << " game " << filedata.getPath() << " not found";
	}
}

const std::string& GameCollection::GetName() const
{
	return m_name;
}

void GameCollection::AddGame(const FileData& filedata)
{
	const std::string& key = GetKey(filedata);
	auto it = mGamesMap.find(key);
	if ( it == mGamesMap.end() )
	{
		mGamesMap.emplace(key, Game(filedata));
	}
	else
	{
		LOG(LogWarning) << "Favorite game " << filedata.getPath() << "already set";
	}
}

void GameCollection::ReplacePlaceholder(const FileData& filedata)
{
	std::string key = GetKey(filedata);
	auto it = mGamesMap.find(key);
	if ( it != mGamesMap.end() )
	{
		it->second = Game(filedata);
	}
}

static const std::string k_gamecollectionTag = "game_collection";
static const std::string k_gamecollectionTagLegacyTag = "favorites";



bool GameCollection::Deserialize(const boost::filesystem::path& folderPath)
{
	pugi::xml_document doc;
	pugi::xml_node root;
	const bool forWrite = false;
	std::string xmlPath = GetFilePath(folderPath);

	if ( boost::filesystem::exists(xmlPath) )
	{
		pugi::xml_parse_result result = doc.load_file(xmlPath.c_str());
		pugi::xml_node root = doc.child(k_gamecollectionTag.c_str());

		if (!root) //legacy tag fallback
		{
			root = doc.child(k_gamecollectionTagLegacyTag.c_str());
		}
		if ( root )
		{
			for (auto const& child : root.children())
			{
				std::string key = child.attribute("key").as_string();
				const Game placeholder;
				mGamesMap.emplace(key, placeholder);
			}
		}
		else
		{
			LOG(LogError) << "Could parsing favorites list: \"" << xmlPath << "\"!";
			return false;
		}
	}
	return true;
}



bool GameCollection::Serialize(const boost::filesystem::path& folderPath)
{
	pugi::xml_document doc;
	pugi::xml_node root;
	const bool forWrite = false;
	std::string xmlPath = GetFilePath(folderPath);

	if ( boost::filesystem::exists(xmlPath) )
	{
		//TODO:: overwrite it?
	}

	root = doc.append_child(k_gamecollectionTag.c_str());
	pugi::xml_attribute attr = root.append_attribute("key");
	attr.set_value(m_name.c_str());

	for ( auto const& keyGamePair : mGamesMap )
	{
		const Game& game = keyGamePair.second;
		if ( game.IsValid() )
		{
			const FileData&  gamedata = game.GetFiledata();
			const std::string key = keyGamePair.first;
			pugi::xml_node newNode = root.append_child("game");
			pugi::xml_attribute attr = newNode.append_attribute("key");
			attr.set_value(key.c_str());
		}
	}
	if ( !doc.save_file(xmlPath.c_str()) )
	{
		LOG(LogError) << "Error saving \"" << xmlPath << "\" (for GameCollection " << m_name << ")!";
		return false;
	}
	return true;
}

bool GameCollection::Serialize()
{
	if (m_folderPath.empty() || !boost::filesystem::exists(m_folderPath))
	{
		return false;
	}
	return Serialize(m_folderPath);
}

bool GameCollection::Deserialize()
{
	if (m_folderPath.empty() || !boost::filesystem::exists(m_folderPath))
	{
		return false;
	}
	return Deserialize(m_folderPath);
}