#include "guis/GuiPagedListView.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"

const uint32_t GuiPagedListView::k_pageEntryCount(6);

GuiPagedListView::GuiPagedListView(
	Window* window,
	const std::string& title,
	OnEntrySelectedCallback callback)
	: GuiOptionWindow(window, title), 
	m_onEntrySelected(callback),
	m_currentPage(0)
{

	mMenu.setSize(Renderer::getScreenWidth() * 0.75f, mMenu.getSize().y());
	mMenu.setPosition(( mSize.x() - mMenu.getSize().x() ) / 2, Renderer::getScreenHeight() * 0.15f);

	m_nextPageButton = mMenu.addButton("Next Page", "Pages", [ this ] { LoadNextPages(1); });
	m_nextBulkPageButton = mMenu.addButton("+10", "Pages", [ this ] { LoadNextPages(10); });

	m_prevPageButton = mMenu.addButton("Prev", "Pages", [ this ] { LoadPrevPages(1); });
	m_prevBulkPageButton = mMenu.addButton("-10", "Pages", [ this ] { LoadPrevPages(10); });

	m_pageCountButton = mMenu.addButton(GetPageLabelText(), "Pages", [ this ] {  });

	m_pageCountButton->setEnabled(false);

	LoadPage(0u);
}

std::string GuiPagedListView::GetPageLabelText() const
{
	return "Page " + std::to_string(m_currentPage+1) + "/" + std::to_string(GetLastPage()+1);
}

uint32_t GuiPagedListView::GetLastPage() const
{
	return static_cast<uint32_t>(std::ceil(m_entries.size() / k_pageEntryCount));
}

bool GuiPagedListView::IsLastPage() const
{
	return m_currentPage == GetLastPage();
}

bool GuiPagedListView::IsFirstPage() const
{
	return m_currentPage == 0u;
}

void GuiPagedListView::LoadPrevPages(uint32_t count)
{
	if (IsFirstPage()) return;
	const uint32_t prevPage = ( count >= m_currentPage ) ? 0u : m_currentPage - count;
	LoadPage(prevPage);
}

void GuiPagedListView::LoadNextPages(uint32_t count)
{
	if (IsLastPage()) return;

	const uint32_t nextPage = std::min(m_currentPage + count, GetLastPage());
	LoadPage(nextPage);
}

void GuiPagedListView::LoadPage(uint32_t page)
{
	mMenu.ClearRows();

	uint32_t rowIndex = std::min(page*k_pageEntryCount, m_entries.size() - k_pageEntryCount);
	uint32_t rowCount = std::min(rowIndex + k_pageEntryCount, m_entries.size());
	for (; rowIndex < rowCount; ++rowIndex)
	{
		InsertRow(*(m_entries[ rowIndex ].get()));
	}
	m_currentPage = page;
	m_pageCountButton->setText(GetPageLabelText(), "Pages");
	m_prevPageButton->setVisible(!IsFirstPage());
	m_nextPageButton->setVisible(!IsLastPage());

	m_nextBulkPageButton->setVisible(!IsLastPage());
	m_prevBulkPageButton->setVisible(!IsFirstPage());
}

void GuiPagedListView::InsertRow(GuiPagedListViewEntry& entry)
{
	ComponentListRow row;
	const std::string title = entry.GetText();
	row.addElement(std::make_shared<TextComponent>(mWindow, title, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = std::bind(&GuiPagedListView::OnRowSelected, 
		this, 
		std::placeholders::_1, 
		std::placeholders::_2, 
		&entry);
	addRow(row);
}

GuiPagedListView::~GuiPagedListView()
{

}

bool GuiPagedListView::OnRowSelected(InputConfig* config, Input input, GuiPagedListViewEntry* entry)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		m_onEntrySelected(entry);
		delete this;
		return true;
	}
	return false;
}

bool GuiPagedListView::input(InputConfig* config, Input input)
{
	if (GuiOptionWindow::input(config, input))
	{
		return true;
	}

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

std::vector<HelpPrompt> GuiPagedListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();

	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("select", "close"));

	return prompts;
}