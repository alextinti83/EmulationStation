#pragma  once
#include "GuiPagedListView.h"

class CfgFile;
class GuiCfgEditor : public GuiPagedListView
{
public:
	GuiCfgEditor(Window* window, const std::string& title, CfgFile& configFile);
	virtual ~GuiCfgEditor();

private:
	void OnEntrySelected(GuiPagedListViewEntry* entry, Window* window);

	CfgFile& m_config;
	static float k_widthSizeScreenPercentage;
};