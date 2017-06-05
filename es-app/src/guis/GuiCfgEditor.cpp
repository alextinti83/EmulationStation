#include "GuiCfgEditor.h"
#include "../CfgFile.h"

class GuiCfgEditorLine : public GuiPagedListViewEntry
{
public:
	GuiCfgEditorLine(std::string line): mLine(line) { }
	virtual std::string GetText() const override { return mLine; }
	std::string mLine;
};


float GuiCfgEditor::k_widthSizeScreenPercentage = 0.9;

GuiCfgEditor::GuiCfgEditor(
	Window* window,
	const std::string& title,
	CfgFile& configFile
	)
	: GuiPagedListView(
		window, 
		title, 
		[] (GuiPagedListViewEntry* entry)
		{ 
			auto cfgEntry = dynamic_cast< GuiCfgEditorLine*>( entry );
			if (cfgEntry)
			{
				
			}
		}, k_widthSizeScreenPercentage)
	, m_config(configFile)
{
	for (CfgEntry& entry : m_config.GetEntries())
	{
		std::unique_ptr<GuiCfgEditorLine> lineEntry(new GuiCfgEditorLine(entry.GetLine()));
		AddEntry(std::move(lineEntry));
	}
	LoadPage(0u);
}

GuiCfgEditor::~GuiCfgEditor()
{

}
