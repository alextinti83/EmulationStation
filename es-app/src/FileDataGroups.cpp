#include "FileDataGroups.h"

#include <sstream>
#include <iomanip>

#include "FileData.h"


const std::set<FileDataGroups::GroupNameT> FileDataGroups::k_groupWhitelist = 
{ "genre", "developer", "players", "publisher", "rating" };



template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out << std::fixed;
	out << std::setprecision(n) << a_value;
	return out.str();
}

bool IsNumber(const std::string& i_number)
{
	char* p;
	float converted = std::strtof(i_number.c_str(), &p);
	return !( *p );
}

std::string fixPrecisionIfNumber(const std::string& i_number)
{
	char* p;
	float converted = std::strtof(i_number.c_str(), &p);
	if (*p)
	{
		return i_number;
	}
	else
	{
		return to_string_with_precision(converted, 1);
	}
}

std::string FormatName(std::string name)
{
	if (name.empty())
	{
		return "Unknown";
	}
	else
	{
		name[ 0 ] = toupper(name[ 0 ]);
		return name;
	}
}

FileDataGroups::GroupsMapT FileDataGroups::buildMetadataGroupsMap(FileData* i_folder)
{
	GroupsMapT metadataGroupsMap;
	GameListT files = i_folder->getFilesRecursive(GAME);
	for (FileData* game : files)
	{
		const GroupNamesMap&  groupNames = game->metadata.GetMetadataMap();
		for (GroupNamesMap::value_type groupPair : groupNames)
		{
			const GroupNameT groupName = groupPair.first;
			if (k_groupWhitelist.find(groupName) != k_groupWhitelist.end())
			{
				const SubGroupNameT subGroupName = fixPrecisionIfNumber(groupPair.second);
				auto groupIt = metadataGroupsMap.find(groupName);
				if (groupIt == metadataGroupsMap.end())
				{
					metadataGroupsMap.emplace(groupName, SubGroupMapT());
				}
				else
				{
					SubGroupMapT& subGroupPair = groupIt->second;
					if (subGroupPair.find(subGroupName) == subGroupPair.end())
					{
						subGroupPair.emplace(subGroupName, GameListT());
					}
				}
				GameListT& games = metadataGroupsMap[ groupName ][ subGroupName ];
				games.push_back(game);
			}
		}
	}
	return metadataGroupsMap;
}



void FileDataGroups::generateGroupFolders(FileData* i_folder)
{
	SystemData* systemData = i_folder->getSystem();
	GroupsMapT metadataGroupsMap = buildMetadataGroupsMap(i_folder);


	FileData* groupsFolder = new FileData(FOLDER, "Game Groups", systemData);
	i_folder->addChild(groupsFolder);
	for (GroupsMapT::value_type group : metadataGroupsMap)
	{
		const std::string groupKey = group.first;
		if (k_groupWhitelist.find(groupKey) != k_groupWhitelist.end())
		{
			const std::string groupName = FormatName(groupKey);
			FileData* groupFolder = new FileData(FOLDER, groupName, systemData);
			groupsFolder->addChild(groupFolder);

			const SubGroupMapT& subGroups = group.second;
			for (SubGroupMapT::value_type subGroup : subGroups)
			{
				std::string subGroupName = FormatName(subGroup.first);
				if (subGroupName == "true")
				{
					subGroupName = "Is " + groupName;
				}
				else if (subGroupName == "false")
				{
					subGroupName = "Is Not " + groupName;
				}
				if (IsNumber(subGroupName))
				{
					subGroupName =  groupName + " " + subGroupName;
				}
				GameListT groupedGames = subGroup.second;
				subGroupName += "    #" + std::to_string(groupedGames.size()) + "";
				// if there's a dot in the filename (such in 1.5), 
				// .5 will be considered as the file's extension
				// and won't be shown.. so we add a fake extension as workaround.
				subGroupName += ".subgroup";

				FileData* subGroupFolder = new FileData(FOLDER, subGroupName, systemData);

				groupFolder->addChild(subGroupFolder);
				for (FileData* game : groupedGames)
				{
					subGroupFolder->addChild(game->Clone());
				}
			}
		}
	}
}