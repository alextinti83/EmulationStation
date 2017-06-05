#include "CfgFile.h"
#include <string>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include <fstream>  
#include "EmulationStation.h"

const std::string CfgFile::k_signaturePrefix = "# Processed by EmulationStation v";
const std::string CfgFile::k_signature = k_signaturePrefix + std::string(PROGRAM_VERSION_STRING) + " #";

bool StartsWith(const std::string& str, const std::string& prefix)
{
	if (str.substr(0, prefix.size()) == prefix)
	{
		return true;
	}
	return false;
}

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
			std::istringstream is_line(line);
			std::string key;
			if (!StartsWith(line, comment) && std::getline(is_line, key, '='))
			{
				std::string value;
				if (std::getline(is_line, value))
				{
					m_cfgEntries.emplace_back(key, value);

				}
				else
				{
					m_cfgEntries.emplace_back("# Error [ValueNotFoundForKey]: " + key);
				}
			}
			else
			{
				m_cfgEntries.emplace_back(line);
			}
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
		return boost::filesystem::remove(m_path);
	}
	return false;
}
bool CfgFile::SaveConfigFile()
{
	return SaveConfigFile(m_path);
}

bool CfgFile::SaveConfigFile(const std::string path)
{
	if (!boost::filesystem::exists(path))
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

std::string CfgEntry::GetLine() const
{
	std::string result;
	if (_key.empty() || _value.empty())
	{
		result = _comment;
	}
	else
	{
		result =  _key + " = " + _value;
		if (!_comment.empty())
		{
			result += " " + _comment;
		}
	}
	return result;
}
