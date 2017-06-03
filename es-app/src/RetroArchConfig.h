#pragma once

class SystemData;
class FileData;
class RetroArchConfig
{
public:
	RetroArchConfig(const SystemData&);
	RetroArchConfig(const FileData&);
};
