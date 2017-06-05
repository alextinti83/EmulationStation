#include "guis/GuiRetroArchOptions.h"
#include "Window.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "GuiRetroArchConfig.h"
#include "SystemData.h"
#include "CfgFile.h"

GuiRetroArchOptions::GuiRetroArchOptions(Window* window, SystemData& system)
	: GuiOptionWindow(window,"RetroArch Config"), mSystem(system)
{
	ComponentListRow row;

	const std::string systemName = strToUpper(mSystem.getName());


	// show filtered menu
	row.elements.clear();
	std::string title = "EDIT " + systemName + " CONFIGURATION";
	row.addElement(std::make_shared<TextComponent>(mWindow, title , Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			std::unique_ptr<CfgFile> retroArchConfig(new CfgFile(""));
			auto title = "EDIT " + boost::filesystem::path(retroArchConfig->GetConfigFilePath()).filename().generic_string();
			auto s = new GuiRetroArchConfig(mWindow, title, mSystem, std::move(retroArchConfig));
			mWindow->pushGui(s);
			return true;
		}
		return false;
	};
	addRow(row);

	
	const FileData* const file = getGamelist()->getCursor();
	const uint32_t maxLength = 30; \
	const std::string origName = file->getName();
	title = "EDIT ";
	title += strToUpper(origName.size() > maxLength ? origName.substr(0, maxLength) + "...": origName);
	title += " CONFIG";
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title, file ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			const std::string cfgpath = file->getPath().generic_string() + ".cfg";
			std::unique_ptr<CfgFile> retroArchConfig(new CfgFile(cfgpath));
			auto title = "EDIT " + boost::filesystem::path(retroArchConfig->GetConfigFilePath()).filename().generic_string();
			auto s = new GuiRetroArchConfig(mWindow, title, mSystem, std::move(retroArchConfig));
			mWindow->pushGui(s);
			return true;
		}
		return false;
	};
	addRow(row);
}

GuiRetroArchOptions::~GuiRetroArchOptions()
{

}

IGameListView* GuiRetroArchOptions::getGamelist()
{
	return ViewController::get()->getGameListView(&mSystem).get();
}