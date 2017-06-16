#include "guis/GuiGameCollections.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include "views/ViewController.h"
#include "SystemData.h"
#include "components/SwitchComponent.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "GuiSettings.h"

#include "GameCollection.h"
#include "GameCollections.h"


void CloseMenu(GuiSettings* menu)
{
	delete menu;
}
enum class GameCollectionOption
{
	New, Duplicate, Rename, Delete, Save, Reload, AddAll
};

GuiGameCollections::GuiGameCollections(
	Window* window,
	SystemData& system,
	GameCollections& gameCollections)
	: GuiOptionWindow(window, "GAME COLLECTIONS")
	, mWindow(window)
	, mSystemData(system)
	, mGameCollections(gameCollections)
	, m_options {
		{ GameCollectionOption::New, "New" },
		{ GameCollectionOption::Duplicate, "Duplicate" },
		{ GameCollectionOption::Rename, "Rename" },
		{ GameCollectionOption::Delete, "Delete" },
		{ GameCollectionOption::Save, "Save" },
		{ GameCollectionOption::Reload, "Reload" },
//		{ GameCollectionOption::AddAll, "Add all games" },
	}
{
	LoadEntries();
}

GuiGameCollections::~GuiGameCollections()
{

}


void GuiGameCollections::LoadEntries()
{
	mMenu.ClearRows();

	GameCollection* current = mGameCollections.GetCurrentGameCollection();

	std::string currentName;
	if (current)
	{
		currentName = current->GetName();
		InsertEntry(currentName);
	}
	for (const GameCollections::GameCollectionMap::value_type& collection : mGameCollections.GetGameCollections())
	{
		if (currentName.empty() || collection.first != current->GetName())
		{
			InsertEntry(collection.first);
		}
	}

	if (current)
	{
		SetCurrent(currentName);
	}
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
		ShowOptionsMenu(selectedEntry);
	}
	return false;
}

std::vector<HelpPrompt> GuiGameCollections::getHelpPrompts()
{
	return {
				{ "a", "Highlight" },
				{ "b", "Back" },
				{ "x", "Options" },
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
	std::string current;
	for (auto& pair : m_entries)
	{
		if (pair.second.switchComponent->getState())
		{
			current = pair.second.key;
		}
	}
	if (key == current)
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
		mGameCollections.SetCurrentGameCollection(key);
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

void GuiGameCollections::ShowOptionsMenu(const GameCollectionEntry selectedEntry)
{
	std::string title = strToUpper(selectedEntry.key);
	auto s = new GuiSettings(mWindow, (title + std::string(" OPTIONS")).c_str());
	std::vector<std::pair<std::shared_ptr<SwitchComponent>, SystemData*>> switches;
	for (const auto& option : m_options)
	{
		InsertRow(
			*s,
			option.second,
			std::bind(&GuiGameCollections::OnOptionSelected,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				option.first,
				selectedEntry, 
				s));
	}
	mWindow->pushGui(s);
}


void GuiGameCollections::InsertRow(GuiSettings& root, const std::string& text, std::function<bool(InputConfig*, Input)> fun)
{
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, text, Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.input_handler = fun;
	root.addRow(row);
}

bool GuiGameCollections::OnOptionSelected(
	InputConfig* config,
	Input input,
	const GameCollectionOption option,
	const GameCollectionEntry selectedEntry,
	GuiSettings* menu)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		GameCollection* collection;
		switch (option)
		{
		case GameCollectionOption::New:
			NewGameCollection(selectedEntry, menu);
			break;
		case GameCollectionOption::Duplicate:
			DuplicateGameCollection(selectedEntry, menu);
			break;
		case GameCollectionOption::Rename:
			RenameGameCollection(selectedEntry, menu);
			break;
		case GameCollectionOption::Delete:
			DeleteGameCollection(selectedEntry, menu);
			break;
		case GameCollectionOption::Save:
			collection = mGameCollections.GetGameCollection(selectedEntry.key);
			if (collection)
			{
				if (collection->Serialize())
				{
					ShowMessage(selectedEntry.key + " saved.");
					ViewController::get()->reloadGameListView(&mSystemData);
				}
				else
				{
					ShowMessage("Error saving " + selectedEntry.key);
				}
			}
			break;
		case GameCollectionOption::Reload:
			collection = mGameCollections.GetGameCollection(selectedEntry.key);
			if (collection)
			{
				if (collection->Deserialize())
				{
					ShowMessage(selectedEntry.key + " reloaded.");
					mGameCollections.ReplaceAllPlacholdersForGameCollection(selectedEntry.key);
					ViewController::get()->reloadGameListView(&mSystemData);
				}
				else
				{
					ShowMessage("Error loading " + selectedEntry.key);
				}
			}
			break;
		case GameCollectionOption::AddAll:
			break;
		default:
			break;
		}
		return true;
	}
	return false;
}

void GuiGameCollections::NewGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu)
{
	const std::size_t count = mGameCollections.GetGameCollections().size();
	const std::string name = "New Collection " + std::to_string(count);
	if (!mGameCollections.NewGameCollection(name))
	{
		ShowMessage("Error while creating the new collection named: " + name + ". Make sure you don't have a collection with this name already.");
	}
	else
	{
		InsertEntry(name);
		CloseMenu(menu);
	}
}

void GuiGameCollections::DuplicateGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu)
{
	const std::string name = selectedEntry.key;
	const std::string duplicateName = name + " Copy";
	if (!mGameCollections.DuplicateGameCollection(name, duplicateName))
	{
		ShowMessage("Error while duplicating the collection " + name + ".");
	}
	else
	{
		InsertEntry(duplicateName);
		CloseMenu(menu);
	}
}

void GuiGameCollections::RenameGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu)
{
	const std::string key = selectedEntry.key;
	mWindow->pushGui(new GuiTextEditPopupKeyboard(mWindow,
		"EDIT COLLECTION NAME", key,
		[ this, key, menu ]
	(const std::string& newKey)
	{
		mGameCollections.RenameGameCollection(key, newKey);
		LoadEntries();
		CloseMenu(menu);
	}, false));
}


void GuiGameCollections::DeleteGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu)
{
	if (mGameCollections.GetGameCollections().size() <= 1)
	{
		ShowMessage("You must keep at least 1 Game Collection.");
	}
	const std::string key = selectedEntry.key;
	ShowQuestion("Are you sure you want to delete " + selectedEntry.key + "?", [ this, key, menu ] ()
	{
		if (!mGameCollections.DeleteGameCollection(key))
		{
			ShowMessage("Could not delete the collection.");
		}
		LoadEntries();
		CloseMenu(menu);
	});
}