#pragma once
#include <string>
class SystemData;

// Loads gamelist.xml data into a SystemData.
void parseGamelist(SystemData* system);
void parseGamelistAtPath(const std::string& xmlpath, SystemData* system);

// Writes currently loaded metadata for a SystemData to gamelist.xml.
void writeGamelistToFile(SystemData* system);
