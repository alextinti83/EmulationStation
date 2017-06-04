#include "guis/GuiImportRetroArchConfig.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"

const uint32_t GuiImportRetroArchConfig::k_pageEntryCount(6);

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

uint32_t GuiImportRetroArchConfig::GetLastPage() const
{
	return std::ceil(m_configPaths.size() / k_pageEntryCount);
}

bool GuiImportRetroArchConfig::IsLastPage() const
{
	return m_currentPage == GetLastPage();
}

bool GuiImportRetroArchConfig::IsFirstPage() const
{
	return m_currentPage == 0u;
}

void GuiImportRetroArchConfig::LoadPrevPages(uint32_t count)
{
	if (IsFirstPage()) return;
	const uint32_t prevPage = ( count >= m_currentPage ) ? 0u : m_currentPage - count;
	LoadPage(prevPage);
}

void GuiImportRetroArchConfig::LoadNextPages(uint32_t count)
{
	if (IsLastPage()) return;

	const uint32_t nextPage = std::min(m_currentPage + count, GetLastPage());
	LoadPage(nextPage);
}

void GuiImportRetroArchConfig::LoadPage(uint32_t page)
{
	mMenu.ClearRows();

	uint32_t rowIndex = std::min(page*k_pageEntryCount, m_configPaths.size() - k_pageEntryCount);
	uint32_t rowCount = std::min(rowIndex + k_pageEntryCount, m_configPaths.size());
	for (; rowIndex < rowCount; ++rowIndex)
	{
		InsertRow(m_configPaths[ rowIndex ]);
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
	
	if (config->isMappedTo("right", input) && input.value != 0)
	{
		LoadNextPages();
		return true;
	}
	if (config->isMappedTo("left", input) && input.value != 0)
	{
		LoadPrevPages();
		return true;
	}

	if (config->isMappedTo("PageUp", input) && input.value != 0)
	{
		LoadNextPages(10);
		return true;
	}
	if (config->isMappedTo("PageDown", input) && input.value != 0)
	{
		LoadPrevPages(10);
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