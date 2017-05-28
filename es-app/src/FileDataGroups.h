#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>



class FileData;

class FileDataGroups
{
public:
	using GroupNameT = std::string; // [Genre, Developer..]
	using SubGroupNameT = std::string; // [Action, RPG..] or [Nintendo, Sega..]
	using GroupNamesMap = std::map<GroupNameT, SubGroupNameT>;
	using GameListT = std::vector<FileData*>;
	using SubGroupMapT = std::unordered_map<SubGroupNameT, GameListT>;
	using GroupsMapT = std::unordered_map<GroupNameT, SubGroupMapT>;

	static GroupsMapT buildMetadataGroupsMap(FileData* i_folder);
	static void generateGroupFolders(FileData* i_folder);
};
