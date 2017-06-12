#include "guis/GuiGameCollections.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"


GuiGameCollections::GuiGameCollections(
	Window* window)
	: GuiOptionWindow(window,  "GAME COLLECTIONS")
		, m_window(window)
{

}

GuiGameCollections::~GuiGameCollections()
{

}
