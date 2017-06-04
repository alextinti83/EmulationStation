#include "guis/GuiImportRetroArchConfig.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"

const uint32_t GuiImportRetroArchConfig::k_pageSize(6);

GuiImportRetroArchConfig::GuiImportRetroArchConfig(
	Window* window,
	const std::string& title,
	const boost::filesystem::path& configFolder,
	CallbackT callback)
	: GuiOptionWindow(window, title), 
	m_configFolder(configFolder), 
	m_onConfigSelected(callback),
	m_currentPage(0)
{
	if (boost::filesystem::exists(configFolder))
	{
		uint32_t count = 0;
		using fsIt = boost::filesystem::recursive_directory_iterator;
		fsIt end;
		for (fsIt i(configFolder); i != end; ++i)
		{
			++count;
			const boost::filesystem::path cp = ( *i );
			m_configPaths.emplace_back(cp);  
			
		}
		if (count == 0)
		{
			window->pushGui(new GuiMsgBox(mWindow,
				"Could not find any configuration in the folder: " + configFolder.generic_string(),
				"Close", [ this ] { delete this; }));
		}
	}

	LoadPage(0u);
}

void GuiImportRetroArchConfig::LoadNextPage()
{
	if (m_currentPage > 0)
	{
		LoadPage(m_currentPage - 1);
	}
}

void GuiImportRetroArchConfig::LoadPrevPage()
{
	if (m_currentPage + k_pageSize < m_configPaths.size())
	{
		LoadPage(m_currentPage + 1);
	}
}

void GuiImportRetroArchConfig::LoadPage(uint32_t page)
{
	mMenu.ClearRows();

	page = std::min(page, m_configPaths.size());
	const uint32_t rowCount = std::min(page + k_pageSize, m_configPaths.size());
	for (uint32_t row = page; row < rowCount; ++row)
	{
		InsertRow(m_configPaths[ row ]);
	}
	m_currentPage = page;
}

void GuiImportRetroArchConfig::InsertRow(boost::filesystem::path path)
{
	ComponentListRow row;
	const std::string title = path.filename().generic_string();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = std::bind(&GuiImportRetroArchConfig::OnRowSelected, this, std::placeholders::_1, std::placeholders::_2, path);
	addRow(row);
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

bool GuiImportRetroArchConfig::input(InputConfig* config, Input input)
{
	
	if (config->isMappedTo("PageUp", input) && input.value != 0)
	{
		LoadPrevPage();
		return true;
	}
	if (config->isMappedTo("PageDown", input) && input.value != 0)
	{
		LoadNextPage();
		return true;
	}

	if (config->isMappedTo("b", input) && input.value != 0)
	{
		delete this;
		return true;
	}

	if (config->isMappedTo("select", input) &&
		input.value != 0)
	{
		// close everything
		Window* window = mWindow;
		while (window->peekGui() && window->peekGui() != ViewController::get())
			delete window->peekGui();
		return true;
	}

	return GuiComponent::input(config, input);
}