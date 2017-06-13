#include "guis/GuiGameCollections.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"
#include "GameCollection.h"
#include "SystemData.h"
#include "components/SwitchComponent.h"


GuiGameCollections::GuiGameCollections(
	Window* window, 
	SystemData& systemData)
	: GuiOptionWindow(window,  "GAME COLLECTIONS")
	, mWindow(window), mSystemData(systemData)
{

	for (const SystemData::GameCollections::value_type& collection : systemData.GetGameCollections())
	{
		InsertEntry(collection.first);
	}
}

GuiGameCollections::~GuiGameCollections()
{

}


void GuiGameCollections::InsertEntry(const std::string& entryName)
{
	const std::string title = entryName;
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, strToUpper(title), Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	auto onOffSwitch = std::make_shared<SwitchComponent>(mWindow);
	onOffSwitch->setState(true);
	row.addElement(onOffSwitch, false, true);

	row.input_handler = std::bind(&GuiGameCollections::OnEntrySelected,
		this,
		std::placeholders::_1,
		std::placeholders::_2,
		entryName, onOffSwitch);
	addRow(row);
}

bool GuiGameCollections::OnEntrySelected(InputConfig* config, Input input, std::string entryname, std::shared_ptr<SwitchComponent> switchComp)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		const bool newState = !switchComp->getState();
		switchComp->setVisible(newState);
		switchComp->setState(newState);

		return true;
	}
	return false;
}