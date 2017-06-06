#include "CfgFile.h"
#include <string>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include <fstream>  
#include "EmulationStation.h"
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

const std::string k_backupFolderName = "retroarch.cfg.bck";

bool CopyConfig(boost::filesystem::path filepath, boost::filesystem::path newFilepath)
{
	boost::system::error_code returnedError;
	boost::filesystem::create_directories(newFilepath.parent_path(), returnedError);
	if (returnedError)
	{
		return false;
	}
	else
	{
		boost::filesystem::copy(filepath, newFilepath);
		return true;
	}
}
bool BackupConfig(boost::filesystem::path filepath, const std::string signature = "")
{
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	const auto date = second_clock::local_time().date();
	const auto time = second_clock::local_time().time_of_day();
	const std::string datePrefix = to_iso_string(date);
	const std::string timePrefix = to_iso_string(time);
	std::string prefix = signature;
	prefix += datePrefix + "_" + timePrefix;
	const auto backupDir = filepath.parent_path();
	const auto filename = filepath.filename().generic_string();
	const auto outputPath = backupDir / k_backupFolderName / ( filename + "." + prefix + ".cfg");
	return CopyConfig(filepath, outputPath);
}

bool StartsWith(const std::string& str, const std::string& prefix)
{
	if (str.substr(0, prefix.size()) == prefix)
	{
		return true;
	}
	return false;
}

CfgEntry::CfgEntry(const std::string line)
{
	SetLine(line);
}

CfgEntry::CfgEntry(const CfgEntry&& other)
	:
	_key(std::move(other._key)),
	_value(std::move(other._value)),
	_comment(std::move(other._comment))
{
}

CfgEntry::~CfgEntry()
{
}

CfgEntry& CfgEntry::operator=(const CfgEntry&& rhs)
{
	_key = std::move(rhs._key);
	_value = std::move(rhs._value);
	_comment = std::move(rhs._comment);
	return *this;
}

std::string CfgEntry::GetLine() const
{
	std::string result;
	if (_key.empty() || _value.empty())
	{
		result = _comment;
	}
	else
	{
		result = _key + " = " + _value;
		if (!_comment.empty())
		{
			result += " " + _comment;
		}
	}
	return result;
}

void CfgEntry::SetLine(const std::string& line)
{
	std::istringstream is_line(line);
	std::string key;
	std::string comment("#");
	if (!StartsWith(line, comment) && std::getline(is_line, key, '='))
	{
		std::string value;
		if (std::getline(is_line, value))
		{
			_key = key;
			_value = value;
		}
		else
		{
			_comment = "# Error [ValueNotFoundForKey]: " + key;
		}
	}
	else
	{
		_comment = line;
	}
}

const bool CfgEntry::IsSignature() const
{
	return StartsWith(_comment, CfgFile::k_signaturePrefix);
}

const std::string CfgFile::k_signaturePrefix = "# Processed by EmulationStation v";
const std::string CfgFile::k_signature = k_signaturePrefix + std::string(PROGRAM_VERSION_STRING) + " #";


CfgFile::CfgFile(const std::string& path)
	: m_path(path)
{
	LoadConfigFile();
}

CfgFile::CfgFile()
{

}

bool CfgFile::LoadConfigFile(const std::string path)
{
	if (boost::filesystem::exists(path))
	{
		m_path = path;
		m_cfgEntries.clear();

		std::ifstream is_file(path);
		std::string comment("#");

		std::string line;
		while (std::getline(is_file, line))
		{
			m_cfgEntries.emplace_back(line);
		}
		return true;
	}
	return false;
}

bool CfgFile::LoadConfigFile()
{
	return LoadConfigFile(m_path);
}

bool CfgFile::ConfigFileExists() const
{
	return boost::filesystem::exists(m_path);
}

bool CfgFile::DeleteConfigFile() const
{
	if (ConfigFileExists())
	{
		BackupConfig(m_path, HasSignature() ? "ES" : "");
		return boost::filesystem::remove(m_path);
	}
	return false;
}

bool CfgFile::SaveConfigFile(bool forceOverwrite)
{
	return SaveConfigFile(m_path, forceOverwrite);
}

bool CfgFile::SaveConfigFile(const std::string path, bool forceOverwrite)
{
	if (!boost::filesystem::exists(path) || forceOverwrite)
	{
		m_path = path;
		UpdateSignature();
		std::ofstream outfile(m_path);
		if (outfile)
		{
			for (const CfgEntry& entry : m_cfgEntries)
			{
				outfile << entry.GetLine() << std::endl;
				if (outfile.bad())
				{
					return false;
				}
			}
			outfile.close();
			return !outfile.bad();
		}
	}
	return false;
}

void CfgFile::UpdateSignature()
{
	bool overwritten = false;
	if (!m_cfgEntries.empty())
	{
		const std::string& firstLine = m_cfgEntries.front().GetLine();
		if (StartsWith(firstLine, k_signaturePrefix))
		{
			m_cfgEntries[ 0 ] = k_signature;
			overwritten = true;
		}
	}
	if (!overwritten)
	{
		m_cfgEntries.emplace(m_cfgEntries.begin(), k_signature);
	}
}

bool CfgFile::HasSignature() const
{
	if (!m_cfgEntries.empty())
	{
		return  m_cfgEntries.front().IsSignature();
	}
	return false;
}

const std::string& CfgFile::GetConfigFilePath() const
{
	return m_path;
}

std::string CfgFile::GetRawText() const
{
	std::string result;
	for ( auto& entry : m_cfgEntries)
	{
		result += entry.GetLine() + "\n";
	}
	return result;
}


