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

	const std::string currentCollectionName = mSystemData.GetCurrentGameCollection()->GetName();
	SetCurrent(currentCollectionName);
}

GuiGameCollections::~GuiGameCollections()
{

}

void GuiGameCollections::InsertEntry(const std::string& key)
{
	GameCollectionEntry row;
	row.key = key;

	row.textComponent = std::make_shared<TextComponent>(mWindow, strToUpper(key), Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
	row.switchComponent = std::make_shared<SwitchComponent>(mWindow);
	
	row.switchComponent->setState(false);
	row.switchComponent->setVisible(false);

	row.addElement(row.textComponent, true);
	row.addElement(row.switchComponent, false, true);

	m_entries.emplace(key, row);
	row.input_handler = std::bind(&GuiGameCollections::OnEntrySelected,
		this,
		std::placeholders::_1,
		std::placeholders::_2,
		GetEntry(key));

	addRow(row);
}


bool GuiGameCollections::OnEntrySelected(InputConfig* config, Input input, 
	GameCollectionEntry* selectedEntry)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		SetCurrent(selectedEntry->key);
		ViewController::get()->reloadGameListView(&mSystemData);
		return true;
	}
	else if (config->isMappedTo("y", input) && input.value)
	{
		const std::size_t count = mSystemData.GetGameCollections().size();
		const std::string name = "New Collection " + std::to_string(count);
		mSystemData.NewGameCollection(name);
		InsertEntry(name);
	}
	return false;
}

GameCollectionEntry* GuiGameCollections::GetEntry(const std::string key)
{
	auto it = m_entries.find(key);
	if (it != m_entries.end())
	{
		return &it->second;
	}
	return nullptr;
}

void GuiGameCollections::SetCurrent(const std::string key)
{
	if (mSystemData.GetCurrentGameCollection()->GetName() == key)
	{
		return;
	}
	for (auto& pair : m_entries)
	{
		pair.second.switchComponent->setVisible(false);
		pair.second.switchComponent->setState(false);
	}
	auto entry = GetEntry(key);
	if (entry)
	{
		entry->switchComponent->setVisible(true);
		entry->switchComponent->setState(true);
		mSystemData.SetCurrentGameCollection(key);
	}
}
