#include "FileDataGroups.h"

#include "FileData.h"


std::string FormatName(std::string name)
{
	if (name.empty())
	{
		return "# Undefined #";
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
			const SubGroupNameT subGroupName = groupPair.second;
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
	return metadataGroupsMap;
}

void FileDataGroups::generateGroupFolders(FileData* i_folder)
{
	SystemData* systemData = i_folder->getSystem();
	GroupsMapT metadataGroupsMap = buildMetadataGroupsMap(i_folder);

	std::set<GroupNameT> groupWhitelist = { "genre", "developer", "players", "publisher", "rating" };

	FileData* groupsFolder = new FileData(FOLDER, "Game Groups", systemData);
	i_folder->addChild(groupsFolder);
	for (GroupsMapT::value_type group : metadataGroupsMap)
	{
		const std::string groupKey = group.first;
		if (groupWhitelist.find(groupKey) != groupWhitelist.end())
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
				FileData* subGroupFolder = new FileData(FOLDER, subGroupName, systemData);
				groupFolder->addChild(subGroupFolder);

				GameListT groupedGames = subGroup.second;
				for (FileData* game : groupedGames)
				{
					subGroupFolder->addChild(game->Clone());
				}
			}
		}
	}
}