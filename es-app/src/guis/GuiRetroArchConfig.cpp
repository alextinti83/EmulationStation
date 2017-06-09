#include "guis/GuiRetroArchConfig.h"
#include "Window.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "CfgFile.h"
#include "guis/GuiMsgBox.h"
#include "GuiImportRetroArchConfig.h"
#include "SystemData.h"
#include "GuiCfgEditor.h"

GuiRetroArchConfig::GuiRetroArchConfig(
	Window* window,
	std::string title,
	SystemData& system,
	std::unique_ptr<CfgFile> config)
	: GuiOptionWindow(window, title),
	mSystem(system),
	m_config(std::move(config)),
	m_backupConfigOptionCreated(false)
{
	const bool configFileExists = m_config->ConfigFileExists();
	if (configFileExists)
	{
		AddEditConfigOption();
		AddDeleteConfigOption();
	}
	//else
	//{
	//	AddCreateConfigOption();
	//}
	AddImportConfigOption(
		mSystem.getRetroArchConfigImportFolder(), 
		"IMPORT CONFIG", "Configuration folder not found: ");

	if (configFileExists)
	{
		AddBackupConfigOption();
	}

	RefreshRestoreBackupConfigOption();
}

GuiRetroArchConfig::~GuiRetroArchConfig()
{
	// nothing to do
}


void GuiRetroArchConfig::AddCreateConfigOption()
{
	ComponentListRow row;
	std::string title = "CREATE CONFIG";
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			auto save = [ this ] (const bool overwrite)
			{
				m_config->SaveConfigFile(overwrite);
				if (!m_config->ConfigFileExists())
				{
					ShowMessage("Could not Save " + m_config->GetConfigFilePath());
				}
				else
				{
					delete this;
				}
			};
			if (m_config->ConfigFileExists())
			{
				mWindow->pushGui(new GuiMsgBox(mWindow, "Do you really want to Overwrite " + m_config->GetConfigFilePath() + "?", "YES",
					[ this, save ]
				{
					const bool overwrite = true;
					save(overwrite);
				}, "NO", nullptr));
			}
			else
			{
				const bool overwrite = false;
				save(overwrite);
			}
			return true;
		}
		return false;
	};
	addRow(row);
}


void GuiRetroArchConfig::AddDeleteConfigOption()
{
	ComponentListRow row;
	std::string title = "DELETE CONFIG";
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			DeleteConfigWithMsg(m_config.get());
		}
		return false;
	};
	addRow(row);
}

void GuiRetroArchConfig::DeleteConfigWithMsg(CfgFile* config, const std::function<void()>& func)
{
	ShowQuestion("Do you really want to Delete " + config->GetConfigFilePath() + "?", 
		[this, config, func ] { DeleteConfig(*config, func); });
}

void GuiRetroArchConfig::DeleteConfigWithMsg(const std::string filepath, const std::function<void()>& func)
{
	ShowQuestion("Do you really want to Delete " + filepath + "?",
		[ this, filepath, func ]
	{ 
		CfgFile cfgFile(filepath);
		DeleteConfig(cfgFile, func);
	});
}
void GuiRetroArchConfig::DeleteConfig(CfgFile& config, const std::function<void()>& func)
{
	config.DeleteConfigFile();
	if (config.ConfigFileExists())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow, "Could not Delete " + config.GetConfigFilePath() + "?", "Close",
			[ this ]
		{
		}, nullptr));
	}
	else
	{
		if (func) { func(); }
		delete this;
	}
}

void GuiRetroArchConfig::AddEditConfigOption()
{
	ComponentListRow row;
	std::string title = "EDIT CONFIG";
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			auto title = boost::filesystem::path(m_config->GetConfigFilePath()).filename().generic_string();
			mWindow->pushGui(new GuiCfgEditor(mWindow, title, *( m_config.get() )));
			return true;
		}
		return false;
	};
	addRow(row);
}

void GuiRetroArchConfig::AddImportConfigOption(
	boost::filesystem::path configFolder, 
	const std::string& title, 
	const std::string& errorMsg,
	const bool addDeleteConfigOption)
{
	ComponentListRow row;
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title, errorMsg, configFolder, addDeleteConfigOption ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			if (boost::filesystem::exists(configFolder))
			{
				auto s = new GuiImportRetroArchConfig(mWindow, title, configFolder,
					std::bind(&GuiRetroArchConfig::OnImportConfigSelected, this, std::placeholders::_1)
				);
				s->SetOnButtonPressedCallback("x", 
					std::bind(&GuiRetroArchConfig::OnImportConfigViewButtonPressed, this, std::placeholders::_1));
				s->SetHelpPrompt(HelpPrompt("x", "View Config"));

				if (addDeleteConfigOption)
				{
					s->SetOnButtonPressedCallback("y",
						std::bind(&GuiRetroArchConfig::OnImportConfigDeleteButtonPressed, this, std::placeholders::_1, s));
					s->SetHelpPrompt(HelpPrompt("y", "Delete Config"));
				}

				mWindow->pushGui(s);
				return true;
			}
			else
			{
				mWindow->pushGui(new GuiMsgBox(mWindow,
					errorMsg + configFolder.generic_string(),
					"Close", [ this ] { delete this; }));
				return true;
			}
		}
		return false;
	};
	addRow(row);
}

void GuiRetroArchConfig::RefreshRestoreBackupConfigOption()
{
	if (m_config->FetchBackups().size() > 0 && !m_backupConfigOptionCreated)
	{
		const bool addDeleteConfigOption = true;
		AddImportConfigOption(m_config->GetBackupFolder(),
			"RESTORE CONFIG BACKUP", "Backup folder not found: ",
			addDeleteConfigOption);
		m_backupConfigOptionCreated = true;
	}
}

void GuiRetroArchConfig::AddBackupConfigOption()
{
	ComponentListRow row;
	std::string title = "BACKUP CONFIG";
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = [ this, title ] (InputConfig* config, Input input)
	{
		if (config->isMappedTo("a", input) && input.value)
		{
			const bool result = m_config->BackupConfig();
			ShowMessage(result ? "Backup successfully created." : "Config backup failed");
			RefreshRestoreBackupConfigOption();
			return true;
		}
		return false;
	};
	addRow(row);
}

IGameListView* GuiRetroArchConfig::getGamelist()
{
	return ViewController::get()->getGameListView(&mSystem).get();
}

void GuiRetroArchConfig::OnImportConfigSelected(boost::filesystem::path configPath)
{
	mWindow->pushGui(new GuiMsgBox(mWindow, "Do you really want to Overwrite " + m_config->GetConfigFilePath() + " with the content of " + configPath.generic_string() + "?", "YES",
		[ this, configPath ]
	{
		std::unique_ptr<CfgFile> new_config(new CfgFile());
		if (LoadConfigFile(new_config, configPath))
		{
			const std::string originalPath = m_config->GetConfigFilePath();
			if (DeleteConfigFile(m_config))
			{
				if (SaveConfigFile(new_config, originalPath))
				{
					m_config = std::move(new_config);
				}
			}
		}
		delete this;
	}, "NO", nullptr));
}

void GuiRetroArchConfig::OnImportConfigViewButtonPressed(boost::filesystem::path configPath)
{
	auto title = boost::filesystem::path(m_config->GetConfigFilePath()).filename().generic_string();
	mWindow->pushGui(new GuiCfgEditor(mWindow, title, *( m_config.get() ), GuiCfgEditor::UILayout::Viewer));
}

void GuiRetroArchConfig::OnImportConfigDeleteButtonPressed(
	boost::filesystem::path configPath, 
	GuiImportRetroArchConfig* importRetroArchConfig)
{
	DeleteConfigWithMsg(configPath.generic_string(),
		[importRetroArchConfig] 
	{
		delete importRetroArchConfig;
	});
}

void GuiRetroArchConfig::ShowMessage(const std::string& mgs, const std::function<void()>& func)
{
	mWindow->pushGui(new GuiMsgBox(mWindow, mgs, "Close", func));
}

void GuiRetroArchConfig::ShowQuestion(const std::string& mgs, const std::function<void()>& func)
{
	mWindow->pushGui(new GuiMsgBox(mWindow, mgs, "YES", func, "NO", nullptr));
}


bool GuiRetroArchConfig::LoadConfigFile(std::unique_ptr<CfgFile>& config, boost::filesystem::path configPath)
{
	const bool result = config->LoadConfigFile(configPath.generic_string());
	if (!result)
	{
		ShowMessage("Could not load " + configPath.generic_string());
	}
	return result;
}


bool GuiRetroArchConfig::SaveConfigFile(std::unique_ptr<CfgFile>& config, boost::filesystem::path configPath)
{
	const bool overwrite = false;
	const bool result = config->SaveConfigFile(configPath.generic_string(), overwrite);
	if (!result)
	{
		ShowMessage("Could not save " + configPath.generic_string());
	}
	return result;
}


bool GuiRetroArchConfig::DeleteConfigFile(std::unique_ptr<CfgFile>& config)
{
	if (config->ConfigFileExists())
	{
		const bool result = config->DeleteConfigFile();
		if (result)
		{
			RefreshRestoreBackupConfigOption(); //delete might trigger an automatic backup
		}
		else
		{
			ShowMessage("Could not delete " + config->GetConfigFilePath());
		}
		return result;
	}
	else
	{
		return true;
	}
}
