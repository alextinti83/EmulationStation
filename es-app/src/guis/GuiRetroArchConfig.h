#pragma  once
#include "guis/GuiOptionWindow.h"

class IGameListView;
class SystemData;
class RetroArchConfig;

class GuiRetroArchConfig : public GuiOptionWindow
{
public:
	GuiRetroArchConfig(Window* window, std::string title,  SystemData&, std::unique_ptr<RetroArchConfig>);
	virtual ~GuiRetroArchConfig();

private:
	IGameListView* getGamelist();
	SystemData& mSystem;
	std::unique_ptr<RetroArchConfig> m_config;
};