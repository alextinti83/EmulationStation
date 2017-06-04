#include "guis/GuiRetroArchConfig.h"
#include "Window.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "CfgFile.h"
#include "guis/GuiMsgBox.h"

GuiRetroArchConfig::GuiRetroArchConfig(
	Window* window, 
	std::string title, 
	SystemData& system, 
	std::unique_ptr<CfgFile> config)
	: GuiOptionWindow(window, title), 
	mSystem(system), 
	m_config(std::move(config))
{
	ComponentListRow row;
	bool configFileExists = m_config->ConfigFileExists();
	if (configFileExists)
	{
		std::string title = "EDIT CONFIG";
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.input_handler = [ this, title ] (InputConfig* config, Input input)
		{
			if (config->isMappedTo("a", input) && input.value)
			{
				//std::unique_ptr<CfgFile> retroArchConfig(new CfgFile(""));
				//auto s = new GuiRetroArchConfig(mWindow, title, mSystem, std::move(retroArchConfig));
				//mWindow->pushGui(s);
				return true;
			}
			return false;
		};
		addRow(row);
		{
			std::string title = "DELETE CONFIG";
			row.elements.clear();
			row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			row.input_handler = [ this, title ] (InputConfig* config, Input input)
			{
				if (config->isMappedTo("a", input) && input.value)
				{
					mWindow->pushGui(new GuiMsgBox(mWindow, "Do you really want to Delete " + m_config->GetConfigFilePath() + "?", "YES",
						[ this ]
					{
						m_config->DeleteConfigFile();
						if (m_config->ConfigFileExists())
						{
							mWindow->pushGui(new GuiMsgBox(mWindow, "Could not Delete " + m_config->GetConfigFilePath() + "?", "Close",
								[ this ]
							{
							}, nullptr));
						} 
						else
						{
							delete this;
						}

					}, "NO", nullptr));
					return true;
				}
				return false;
			};
			addRow(row);
		}
	}
	else
	{
		std::string title = "CREATE CONFIG";
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.input_handler = [ this, title ] (InputConfig* config, Input input)
		{
			if (config->isMappedTo("a", input) && input.value)
			{
				bool writeFile = true;
				if (m_config->ConfigFileExists())
				{
					mWindow->pushGui(new GuiMsgBox(mWindow, "Do you really want to Overwrite " + m_config->GetConfigFilePath() + "?", "YES",
						[ this, &writeFile ]
					{
						writeFile = true;
					}, "NO", nullptr));
				}
				if (writeFile)
				{
					m_config->SaveConfigFile();
					if (!m_config->ConfigFileExists())
					{
						mWindow->pushGui(new GuiMsgBox(mWindow, "Could not Save " + m_config->GetConfigFilePath() + "?", "Close",
							[ this ]
						{
						}, nullptr));
					}
					else
					{
						delete this;
					}
				}
				return true;
			}
			return false;
		};
		addRow(row);
	}

	{
		std::string title = "IMPORT CONFIG";
		row.elements.clear();
		row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		row.input_handler = [ this, title ] (InputConfig* config, Input input)
		{
			if (config->isMappedTo("a", input) && input.value)
			{
				//std::unique_ptr<CfgFile> retroArchConfig(new CfgFile(""));
				//auto s = new GuiRetroArchConfig(mWindow, title, mSystem, std::move(retroArchConfig));
				//mWindow->pushGui(s);
				return true;
			}
			return false;
		};
		addRow(row);
	}
}

GuiRetroArchConfig::~GuiRetroArchConfig()
{

}

IGameListView* GuiRetroArchConfig::getGamelist()
{
	return ViewController::get()->getGameListView(&mSystem).get();
}
