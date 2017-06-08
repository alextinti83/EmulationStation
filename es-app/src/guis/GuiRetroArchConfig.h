#pragma  once
#include "guis/GuiOptionWindow.h"

class IGameListView;
class SystemData;
class CfgFile;
class GuiImportRetroArchConfig;

class GuiRetroArchConfig : public GuiOptionWindow
{
public:
	GuiRetroArchConfig(Window* window, std::string title, SystemData&,std::unique_ptr<CfgFile>);
	virtual ~GuiRetroArchConfig();

private:
	void AddCreateConfigOption();
	void AddDeleteConfigOption();
	void DeleteConfigWithMsg(CfgFile* m_config, const std::function<void()>& func = nullptr);
	void DeleteConfigWithMsg(const std::string filepath, const std::function<void()>& func = nullptr);
	void DeleteConfig(CfgFile& config, const std::function<void()>& func = nullptr);
	void AddEditConfigOption();
	void AddImportConfigOption(
		const boost::filesystem::path configFolder, 
		const std::string& title, 
		const std::string& errorMsg,
		const bool addDeleteConfigOption = false);
	void RefreshRestoreBackupConfigOption();
	void AddBackupConfigOption();

	IGameListView* getGamelist();
	void OnImportConfigSelected(boost::filesystem::path configPath);
	void OnImportConfigViewButtonPressed(boost::filesystem::path configPath);
	void OnImportConfigDeleteButtonPressed(boost::filesystem::path configPath, GuiImportRetroArchConfig*);
	bool LoadConfigFile(std::unique_ptr<CfgFile>& config, boost::filesystem::path configPath);
	bool SaveConfigFile(std::unique_ptr<CfgFile>& config, boost::filesystem::path configPath);
	bool DeleteConfigFile(std::unique_ptr<CfgFile>& config);
	void ShowMessage(const std::string& mgs, const std::function<void()>& func1 = nullptr);
	void ShowQuestion(const std::string& mgs, const std::function<void()>& func);

	SystemData& mSystem;
	std::unique_ptr<CfgFile> m_config;
	bool m_backupConfigOptionCreated;
};