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
	IGameListView* getGamelist();
	SystemData& mSystem;
	std::unique_ptr<CfgFile> m_config;
};