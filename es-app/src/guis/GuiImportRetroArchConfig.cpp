#include "guis/GuiImportRetroArchConfig.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"

GuiImportRetroArchConfig::GuiImportRetroArchConfig(
	Window* window,
	const std::string& title,
	const boost::filesystem::path& configFolder,
	CallbackT callback)
	: GuiOptionWindow(window, title), m_configFolder(configFolder), m_onConfigSelected(callback)
{
	if (boost::filesystem::exists(configFolder))
	{
		uint32_t count = 0;
		ComponentListRow row;
		using fsIt = boost::filesystem::recursive_directory_iterator;
		fsIt end;
		for (fsIt i(configFolder); i != end; ++i)
		{
			++count;
			const boost::filesystem::path cp = ( *i );
			const std::string title = cp.filename().generic_string();
			row.elements.clear();
			row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
			row.input_handler = std::bind(&GuiImportRetroArchConfig::OnRowSelected, this, std::placeholders::_1, std::placeholders::_2, cp);
			addRow(row);
		}
		if (count == 0)
		{
			window->pushGui(new GuiMsgBox(mWindow,
				"Could not find any configuration in the folder: " + configFolder.generic_string(),
				"Close", [ this ] { delete this; }));

		}
	}
}

GuiImportRetroArchConfig::~GuiImportRetroArchConfig()
{

}

bool GuiImportRetroArchConfig::OnRowSelected(InputConfig* config, Input input, boost::filesystem::path configPath)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		m_onConfigSelected(configPath);
		delete this;
		return true;
	}
	return false;
}