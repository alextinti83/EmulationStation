#include "guis/GuiRetroArchConfig.h"
#include "Window.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "RetroArchConfig.h"

GuiRetroArchConfig::GuiRetroArchConfig(Window* window, std::string title, SystemData& system, std::unique_ptr<RetroArchConfig> config)
	: GuiOptionWindow(window, title), mSystem(system), m_config(std::move(config))
{
	ComponentListRow row;


}

GuiRetroArchConfig::~GuiRetroArchConfig()
{

}

IGameListView* GuiRetroArchConfig::getGamelist()
{
	return ViewController::get()->getGameListView(&mSystem).get();
}
