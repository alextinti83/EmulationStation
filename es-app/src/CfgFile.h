#pragma once
#include <vector>
#include <string>
#include <assert.h>
#include "boost/filesystem/string_file.hpp"
class CfgEntry
{
public:
	CfgEntry(const std::string line);
	~CfgEntry();
	CfgEntry(const CfgEntry&) = delete;
	CfgEntry(const CfgEntry&& other);
	CfgEntry& operator=(const CfgEntry&& rhs);;

	CfgEntry& operator=(const CfgEntry& rhs) = delete;

	std::string GetLine() const;
	void SetLine(const std::string& line);
	const bool IsCommentOnly() const { return !_comment.empty() && _key.empty() && _value.empty(); }
	const bool IsEmpty() const { return _comment.empty() && _key.empty() && _value.empty(); }
	const bool IsSignature() const;

private:
	std::string _key;
	std::string _value;
	std::string _comment;
};

class CfgFile
{
public:
	CfgFile();
	CfgFile(const std::string& path);
	bool LoadConfigFile();
	bool LoadConfigFile(const std::string path);

	bool ConfigFileExists() const;
	bool DeleteConfigFile() const;
	bool SaveConfigFile(bool forceOverwrite);
	bool SaveConfigFile(const std::string path, bool forceOverwrite);
	const std::string& GetConfigFilePath() const;
	std::string GetRawText() const;
	std::vector<CfgEntry>& GetEntries() { return m_cfgEntries; }
	const std::vector<CfgEntry>& GetEntries() const { return m_cfgEntries; }
	boost::filesystem::path GetBackupFolder() const;
	std::vector<boost::filesystem::path> FetchBackups() const;

	static const std::string k_signaturePrefix;
private:
	void UpdateSignature();
	bool HasSignature() const;
	std::vector<CfgEntry> m_cfgEntries;
	std::string m_path;
	static const std::string k_signature;
};