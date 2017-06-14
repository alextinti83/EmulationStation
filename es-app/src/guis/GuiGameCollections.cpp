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
	LoadEntries();
}

GuiGameCollections::~GuiGameCollections()
{

}


void GuiGameCollections::LoadEntries()
{
	mMenu.ClearRows();
	for (const SystemData::GameCollections::value_type& collection : mSystemData.GetGameCollections())
	{
		InsertEntry(collection.first);
	}
	const std::string currentCollectionName = mSystemData.GetCurrentGameCollection()->GetName();
	SetCurrent(currentCollectionName);
}

void GuiGameCollections::InsertEntry(const std::string& key)
{
	GameCollectionEntry entry;
	ComponentListRow row;
	entry.key = key;

	entry.textComponent = std::make_shared<TextComponent>(mWindow, key, Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
	entry.switchComponent = std::make_shared<SwitchComponent>(mWindow);
	
	entry.switchComponent->setState(false);
	entry.switchComponent->setVisible(false);

	row.addElement(entry.textComponent, true);
	row.addElement(entry.switchComponent, false, true);

	m_entries.emplace(key, entry);

	row.input_handler = std::bind(&GuiGameCollections::OnEntrySelected,
		this,
		std::placeholders::_1,
		std::placeholders::_2,
		entry);

	addRow(row);
	m_entriesIndex.emplace(key, mMenu.GetEntryCount()-1);
}

bool GuiGameCollections::OnEntrySelected(InputConfig* config, Input input, 
	GameCollectionEntry selectedEntry)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		SetCurrent(selectedEntry.key);
		ViewController::get()->reloadGameListView(&mSystemData);
		return true;
	}
	else if (config->isMappedTo("x", input) && input.value)
	{
		const std::size_t count = mSystemData.GetGameCollections().size();
		const std::string name = "New Collection " + std::to_string(count);
		if (!mSystemData.NewGameCollection(name))
		{
			ShowMessage("Error while creating the new collection named: " + name + ". Make sure you don't have a collection with this name already.");
		}
		else
		{
			InsertEntry(name);
		}
	}
	else if (config->isMappedTo("y", input) && input.value)
	{
		return true;
	}
	else if (config->isMappedTo("select", input) && input.value)
	{
		const std::string key = selectedEntry.key;
		ShowQuestion("Are you sure you want to delete " + selectedEntry.key + "?", [ this, key ] ()
		{
			if (!mSystemData.DeleteGameCollection(key))
			{
				ShowMessage("Could not delete the collection.");
			}
			LoadEntries();
		});
		return true;
	}
	else if (config->isMappedTo("start", input) && input.value)
	{
		return true;
	}
	return false;
}

std::vector<HelpPrompt> GuiGameCollections::getHelpPrompts()
{
	return {
				{ "a", "Highlight" },
				{ "b", "Back" },
				{ "x", "New" },
				{ "y", "Raname" },
				{ "start", "Hide" },
				{ "select", "Delete"}
			};;
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
	auto& it = m_entriesIndex.find(key);
	if (it != m_entriesIndex.end() && mMenu.GetCursor() == it->second)
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

void GuiGameCollections::ShowMessage(const std::string& mgs, const std::function<void()>& func)
{
	mWindow->pushGui(new GuiMsgBox(mWindow, mgs, "Close", func));
}

void GuiGameCollections::ShowQuestion(const std::string& mgs, const std::function<void()>& func)
{
	mWindow->pushGui(new GuiMsgBox(mWindow, mgs, "YES", func, "NO", nullptr));
}

