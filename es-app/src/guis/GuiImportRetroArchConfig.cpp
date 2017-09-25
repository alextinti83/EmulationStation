#include "guis/GuiImportRetroArchConfig.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"

class GuiImportRetroArchConfigEntry : public GuiPagedListViewEntry
{
public:
	GuiImportRetroArchConfigEntry(boost::filesystem::path root, boost::filesystem::path path): mPath(path), mRoot(root) { }
	virtual std::string GetText() const override 
	{
		const bool allowHome = false;
		std::string path = mPath.generic_string();
		std::string root = mRoot.generic_string();
		std::replace(path.begin(), path.end(), '\\', '/');
		std::replace(root.begin(), root.end(), '\\', '/');
		std::string::iterator it = root.end() - 1;
		if (*it == '/') { root.erase(it); }
		std::string result = makeRelativePath(path, root, allowHome).generic_string();
		it = result.begin();
		if (*it == '.') { result.erase(it); }
		if (*it == '/') { result.erase(it); }
		return result;
	}
	boost::filesystem::path mPath;
	boost::filesystem::path mRoot;

};

GuiImportRetroArchConfig::GuiImportRetroArchConfig(
	Window* window,
	const std::string& title,
	const boost::filesystem::path& configFolder,
	CallbackT callback)
	: GuiPagedListView(window, title, 
		[ callback ] (GuiPagedListViewEntry* entry)
		{ 
			auto cfgEntry = dynamic_cast< GuiImportRetroArchConfigEntry*>( entry );
			if (cfgEntry)
			{
				callback(cfgEntry->mPath);
			}
		}), m_configFolder(configFolder), m_window(window)
{
	ReloadConfigs();
}

GuiImportRetroArchConfig::~GuiImportRetroArchConfig()
{

}

void GuiImportRetroArchConfig::SetOnButtonPressedCallback(const std::string& button, CallbackT callback)
{
	GuiPagedListView::SetOnButtonPressedCallback(button, [callback] (GuiPagedListViewEntry* entry)
	{
		auto cfgEntry = dynamic_cast< GuiImportRetroArchConfigEntry* >( entry );
		if (cfgEntry)
		{
			callback(cfgEntry->mPath);
		}
	});
}

void GuiImportRetroArchConfig::ReloadConfigs()
{
	ClearEntries();
	if (boost::filesystem::exists(m_configFolder))
	{
		uint32_t count = 0;
		using fsIt = boost::filesystem::recursive_directory_iterator;
		fsIt end;
		for (fsIt i(m_configFolder); i != end; ++i)
		{
			++count;
			const boost::filesystem::path cp = ( *i );
			if (!boost::filesystem::is_directory(cp))
			{
				std::unique_ptr<GuiImportRetroArchConfigEntry> entry(new GuiImportRetroArchConfigEntry(m_configFolder, cp));
				AddEntry(std::move(entry));
			}

		}
		if (count == 0)
		{
			m_window->pushGui(new GuiMsgBox(mWindow,
				"Could not find any configuration in the folder: " + m_configFolder.generic_string(),
				"Close", [ this ] { delete this; }));
		}
	}
	SortEntriesFunc alphabetize = [](
		const std::unique_ptr<GuiPagedListViewEntry>& lhs,
		const std::unique_ptr<GuiPagedListViewEntry>& rhs)
	{
		return lhs->GetText() < rhs->GetText();
	}; SortEntries(alphabetize);

	LoadPage(0u);
}
