#pragma  once
#include "GuiPagedListView.h"

class CfgFile;
class GuiCfgEditor : public GuiPagedListView
{
public:
	GuiCfgEditor(Window* window, const std::string& title, CfgFile& configFile);
	virtual ~GuiCfgEditor();
private:
	CfgFile& m_config;

	static float k_widthSizeScreenPercentage;
};