#pragma  once
#include "guis/GuiOptionWindow.h"
#include "components/TextListComponent.h"

class GuiImportRetroArchConfig : public GuiOptionWindow
{
public:
	using CallbackT = std::function<void(boost::filesystem::path)>;
	GuiImportRetroArchConfig(Window* window, const std::string& title, const boost::filesystem::path& configFolder, CallbackT);
	virtual ~GuiImportRetroArchConfig();

private:
	uint32_t GetLastPage() const;
	bool IsLastPage() const;
	bool IsFirstPage() const;
	void LoadNextPages(uint32_t count = 1);
	void LoadPrevPages(uint32_t count = 1);
	void LoadPage(uint32_t page);
	void InsertRow(boost::filesystem::path path);
	bool OnRowSelected(InputConfig* config, Input input,  boost::filesystem::path);
	
	bool input(InputConfig* config, Input input) override;

	boost::filesystem::path m_configFolder;
	CallbackT m_onConfigSelected;
	std::vector<boost::filesystem::path> m_configPaths;
	uint32_t m_currentPage;
	static const uint32_t k_pageEntryCount;

};