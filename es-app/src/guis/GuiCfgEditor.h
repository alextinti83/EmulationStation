#pragma  once
#include "guis/GuiPagedListView.h"

class CfgFile;
class ButtonComponent;
class CfgEntry;
class GuiCfgEditor : public GuiPagedListView
{
public:
	enum class UILayout { Editor, Viewer };

	GuiCfgEditor(Window* window, const std::string& title, CfgFile& configFile, UILayout layout = UILayout::Editor);
	virtual ~GuiCfgEditor();

private:
	void ReloadEntries();
	void OnEntrySelected(GuiPagedListViewEntry* entry, Window* window);
	void OnLineChanged(const std::string& line, CfgEntry* entry);
	void OnSaveButtonPressed();

	CfgFile& m_config;
	std::shared_ptr<ButtonComponent> mSaveButton;
	std::shared_ptr<ButtonComponent> mCommentButton;

	bool mShowComments;
	UILayout m_layout;

	static float k_widthSizeScreenPercentage;
};