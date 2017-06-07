#pragma  once
#include "guis/GuiOptionWindow.h"

class IGameListView;
class SystemData;
class CfgFile;

class GuiRetroArchConfig : public GuiOptionWindow
{
public:
	GuiRetroArchConfig(Window* window, std::string title, SystemData&,std::unique_ptr<CfgFile>);
	virtual ~GuiRetroArchConfig();

private:
	void AddCreateConfigOption();
	void AddDeleteConfigOption();
	void AddEditConfigOption();
	void AddImportConfigOption(
		const boost::filesystem::path configFolder, 
		const std::string& title, 
		const std::string& errorMsg);

	IGameListView* getGamelist();
	void OnImportConfigSelected(boost::filesystem::path configPath);
	void OnImportConfigViewButtonPressed(boost::filesystem::path configPath);
	bool LoadConfigFile(std::unique_ptr<CfgFile>& config, boost::filesystem::path configPath);
	bool SaveConfigFile(std::unique_ptr<CfgFile>& config, boost::filesystem::path configPath);
	bool DeleteConfigFile(std::unique_ptr<CfgFile>& config);
	void ShowError(std::string mgs);

	SystemData& mSystem;
	std::unique_ptr<CfgFile> m_config;
};