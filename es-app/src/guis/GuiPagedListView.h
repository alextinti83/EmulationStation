#pragma  once
#include "guis/GuiOptionWindow.h"
#include "components/ButtonComponent.h"

class GuiPagedListViewEntry
{
public:
	virtual std::string GetText() const = 0;
};

class GuiPagedListView : public GuiOptionWindow
{
public:
	using OnEntrySelectedCallback = std::function<void(GuiPagedListViewEntry*)>;
	GuiPagedListView(Window* window, const std::string& title, OnEntrySelectedCallback, float widthSizePerc = 0.75f);
	virtual ~GuiPagedListView();

	void AddEntry(std::unique_ptr<GuiPagedListViewEntry> entry)
	{
		m_entries.push_back(std::move(entry));
	}
	void ClearEntries()
	{
		m_entries.clear();
	}
	using SortEntriesFunc = std::function<
		bool(
			const std::unique_ptr<GuiPagedListViewEntry>&,
			const std::unique_ptr<GuiPagedListViewEntry>&)>;
	void SortEntries(SortEntriesFunc fun);

	void LoadPage(uint32_t page);
	void ReloadCurrentPage() { LoadPage(m_currentPage); }
	
private:
	void OnButtonAdded(std::shared_ptr<ButtonComponent> button) override;
	void InsertRow(GuiPagedListViewEntry& entry);
	std::string GetPageLabelText() const;
	uint32_t GetLastPage() const;
	bool IsLastPage() const;
	bool IsFirstPage() const;
	void LoadNextPages(uint32_t count = 1);
	void LoadPrevPages(uint32_t count = 1);
	bool OnRowSelected(InputConfig* config, Input input, GuiPagedListViewEntry*);
	bool IsAnyOfMyButtonsFocused() const;
	
	bool input(InputConfig* config, Input input) override;

	std::vector<HelpPrompt> getHelpPrompts() override;
	boost::filesystem::path m_configFolder;
	OnEntrySelectedCallback m_onEntrySelected;
	uint32_t m_currentPage;
	static const uint32_t k_pageEntryCount;

	std::shared_ptr<ButtonComponent> m_pageCountButton;
	std::shared_ptr<ButtonComponent> m_nextPageButton;
	std::shared_ptr<ButtonComponent> m_prevPageButton;
	std::shared_ptr<ButtonComponent> m_nextBulkPageButton;
	std::shared_ptr<ButtonComponent> m_prevBulkPageButton;
	std::vector < std::shared_ptr<ButtonComponent>> m_pageButtons;
	std::vector<std::unique_ptr<GuiPagedListViewEntry>> m_entries;

};