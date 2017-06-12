#pragma  once
#include "GuiOptionWindow.h"

class GuiGameCollections : public GuiOptionWindow
{
public:
	GuiGameCollections(Window* window);
	virtual ~GuiGameCollections();

private:
	Window* m_window;
};