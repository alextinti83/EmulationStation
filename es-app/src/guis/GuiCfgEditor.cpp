#include "GuiCfgEditor.h"
#include "../CfgFile.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "Window.h"

class GuiCfgEditorLine : public GuiPagedListViewEntry
{
public:
	GuiCfgEditorLine(CfgEntry entry): mEntry(entry) { }
	virtual std::string GetText() const override { return mEntry.GetLine(); }
	CfgEntry mEntry;
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
		[this, window ] (GuiPagedListViewEntry* entry)
		{ 
			auto cfgEntry = dynamic_cast< GuiCfgEditorLine*>( entry );
			if (cfgEntry)
			{
				const std::string line = cfgEntry->mEntry.GetLine();
				auto updateVal = [ cfgEntry ] (const std::string& newVal) 
				{ 
					cfgEntry->mEntry.SetLine(newVal);
				};
				window->pushGui(new GuiTextEditPopupKeyboard(mWindow,
						"EDIT LINE",
						line,
						updateVal,
						false));
			}
		}, k_widthSizeScreenPercentage)
	, m_config(configFile)
{
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
