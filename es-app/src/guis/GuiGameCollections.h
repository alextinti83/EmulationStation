#pragma  once
#include "GuiOptionWindow.h"
class SystemData;
class SwitchComponent;

class GuiGameCollections : public GuiOptionWindow
{
public:
	GuiGameCollections(Window* window, SystemData& mSystemData);
	virtual ~GuiGameCollections();

	void InsertEntry(const std::string& entryName);
private:
	bool OnEntrySelected(InputConfig* config, Input input, std::string entryname, std::shared_ptr<SwitchComponent> switchComp);

	Window* mWindow;
	SystemData& mSystemData;
};