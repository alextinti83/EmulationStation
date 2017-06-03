#pragma  once
#include "guis/GuiOptionWindow.h"

class IGameListView;
class SystemData;
class GuiRetroArchOptions : public GuiOptionWindow
{
public:
	GuiRetroArchOptions(Window* window, SystemData&);
	virtual ~GuiRetroArchOptions();

private:
	IGameListView* getGamelist();
	void ShowSelectedGameConfig();

	SystemData& mSystem;
};