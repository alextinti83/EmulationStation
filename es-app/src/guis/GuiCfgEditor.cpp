#include "GuiCfgEditor.h"
#include "../CfgFile.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "Window.h"
#include "components/ButtonComponent.h"
#include "guis/GuiMsgBox.h"

class GuiCfgEditorLine : public GuiPagedListViewEntry
{
public:
	GuiCfgEditorLine(CfgEntry& entry): mEntry(entry) { }
	virtual std::string GetText() const override { return mEntry.GetLine(); }
	CfgEntry& mEntry;
};


float GuiCfgEditor::k_widthSizeScreenPercentage = 0.9f;

GuiCfgEditor::GuiCfgEditor(
	Window* window,
	const std::string& title,
	CfgFile& configFile
	)
	: GuiPagedListView(
		window, 
		title, 
		std::bind(&GuiCfgEditor::OnEntrySelected, this, std::placeholders::_1, window),
		k_widthSizeScreenPercentage)
	, m_config(configFile)
{

	m_config.LoadConfigFile();
	mSaveButton = addButton("Save", "Pages", std::bind(&GuiCfgEditor::OnSaveButtonPressed, this));

	for (CfgEntry& entry : m_config.GetEntries())
	{
		std::unique_ptr<GuiCfgEditorLine> lineEntry(new GuiCfgEditorLine(entry));
		AddEntry(std::move(lineEntry));
	}
	LoadPage(0u);
}

GuiCfgEditor::~GuiCfgEditor()
{

}

void GuiCfgEditor::OnEntrySelected(GuiPagedListViewEntry* entry, Window* window)
{
	auto cfgEntry = dynamic_cast< GuiCfgEditorLine* >( entry );
	if (cfgEntry)
	{
		const std::string line = cfgEntry->mEntry.GetLine();
		CfgEntry* entryPtr = &cfgEntry->mEntry;
		window->pushGui(new GuiTextEditPopupKeyboard(mWindow,
			"EDIT LINE",
			line,
			std::bind(&GuiCfgEditor::OnLineChanged, this, std::placeholders::_1, entryPtr),
			false));
	}
}
void GuiCfgEditor::OnLineChanged(const std::string& line, CfgEntry* entry)
{
	entry->SetLine(line);
	ReloadCurrentPage();
}



void GuiCfgEditor::OnSaveButtonPressed()
{
	mWindow->pushGui(new GuiMsgBox(mWindow,
		"Do you really want to Save " + m_config.GetConfigFilePath() + "?", "YES",
		[ this ]
	{
		const bool overwrite = true;
		if (!m_config.SaveConfigFile(overwrite))
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, "Could not Save file: " + m_config.GetConfigFilePath(),
				"Close", [] { }));
		}
	}));
}
