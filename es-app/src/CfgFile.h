#pragma once
#include <vector>
#include <string>
#include <assert.h>
class CfgEntry
{
public:
	CfgEntry(const std::string& comment) : _comment(comment) { }
	CfgEntry(const std::string& key, std::string value) : _key(key), _value(value) { }
	~CfgEntry() { }
	std::string GetLine() const;
	//enum class Type { k_comment, k_bool, k_int, k_real, k_string };
	//CfgEntry(const std::string& key, bool value)		: _key(key), _type(Type::k_bool),	_bool(value) { }
	//CfgEntry(const std::string& key, int value)			: _key(key), _type(Type::k_int),	_int(value)	 { }
	//CfgEntry(const std::string& key, double value)		: _key(key), _type(Type::k_real),	_real(value) { }
	//
	//bool IsType(Type type) const { return type == _type; }
	//
	//const std::string& GetComment() const 
	//{ 
	//	assert(IsType(Type::k_comment));
	//	return IsType(Type::k_comment) ?_string : "";
	//}
	//const std::string& Getkey() const
	//{
	//	assert(!IsType(Type::k_comment));
	//	return !IsType(Type::k_comment) ? _key : "";
	//}
	//const std::string& GetString() const
	//{
	//	assert(IsType(Type::k_string));
	//	return IsType(Type::k_string) ? _string : "";
	//}
	//const int GetInt() const
	//{
	//	assert(IsType(Type::k_int));
	//	return IsType(Type::k_int) ? _int : 0;
	//}
	//const double GetReal() const
	//{
	//	assert(IsType(Type::k_real));
	//	return IsType(Type::k_real) ? _real : 0.0;
	//}
	//const bool GetBool() const
	//{
	//	assert(IsType(Type::k_bool));
	//	return IsType(Type::k_bool) ? _bool : false;
	//}

private:
	std::string _key;
	std::string _value;
	std::string _comment;
};



class CfgFile
{
public:
	CfgFile(const std::string& path);
	void LoadConfigFile();
	void LoadConfigFile(const std::string path);

	bool ConfigFileExists() const;
	void DeleteConfigFile() const;
	void SaveConfigFile() const;
	const std::string& GetConfigFilePath() const;
private:
	std::vector<CfgEntry> m_cfgEntries;
	std::string m_path;
};
