#pragma  once
#include "GuiPagedListView.h"

class CfgFile;
class ButtonComponent;
class CfgEntry;
class GuiCfgEditor : public GuiPagedListView
{
public:
	GuiCfgEditor(Window* window, const std::string& title, CfgFile& configFile);
	virtual ~GuiCfgEditor();

private:
	void OnEntrySelected(GuiPagedListViewEntry* entry, Window* window);
	void OnLineChanged(const std::string& line, CfgEntry* entry);
	void OnSaveButtonPressed();

	CfgFile& m_config;
	std::shared_ptr<ButtonComponent> mSaveButton;
	static float k_widthSizeScreenPercentage;
};