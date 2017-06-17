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
#include "components/OptionListComponent.h"


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
		{ GameCollectionOption::Save, "Save" },
		{ GameCollectionOption::Reload, "Reload" },
		{ GameCollectionOption::Delete, "Delete" },
//		{ GameCollectionOption::AddAll, "Add all games" },
	}
{
	LoadEntries();
}

GuiGameCollections::~GuiGameCollections()
{
	for (auto& f : m_onCloseFunctions)	{ f(); }
	if (m_gamelistNeedsReload)
	{
		ViewController::get()->reloadGameListView(&mSystemData);
	}
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
	const GameCollection* gc = mGameCollections.GetGameCollection(key);
	if (gc)
	{
		GameCollectionEntry entry;
		auto tags = std::make_shared< OptionListComponent<GameCollection::Tag> >(mWindow, "Tags", false);
		entry.optionListComponent = tags;

		for (GameCollection::Tag tag : GameCollection::GetTags())
		{
			const bool selected = gc->HasTag(tag);
			const std::string tagName = GameCollection::GetTagName(tag);
			tags->add(tagName, tag, selected);
		}
		AddOnCloseFunction([ this, tags, key ]
		{
			GameCollection* gc = mGameCollections.GetGameCollection(key);
			if (gc)
			{
				const GameCollection::Tag selectedTag = tags->getSelected();
				if (!gc->HasTag(selectedTag))
				{
					gc->SetTag(selectedTag);
					m_gamelistNeedsReload = true;
				}
			}
		});
		
		ComponentListRow row;
		entry.key = key;
		auto padding = std::make_shared<TextComponent>(mWindow, "  ", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		entry.textComponent = std::make_shared<TextComponent>(mWindow, key, Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		entry.switchComponent = std::make_shared<SwitchComponent>(mWindow);

		entry.switchComponent->setState(false);
		entry.switchComponent->setVisible(false);
		
		const bool resizeWith = true;
		const bool invertWhenSelected = true;

		row.addElement(entry.switchComponent, !resizeWith, invertWhenSelected);
		row.addElement(padding, !resizeWith);
		row.addElement(entry.textComponent, resizeWith);
		row.addElement(tags, !resizeWith, invertWhenSelected);


		// The current ComponentList behavior does NOT forward the input to 
		// its rightmost element if we set an inputHandler
		// It doesn't care whether our inputHandler actually consumes the event or discards it..
		// I'd rather fix the ComponentList but I'm afraid that will cause 
		// some undesired side effects
		// So.. here's as workaround we pass the OptionListComponent to our
		// input handler so that we can eventually forward the input to it.
		row.input_handler = std::bind(&GuiGameCollections::OnEntrySelected,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			entry);
		
		m_entries.emplace(key, entry);
		addRow(row);
	}
}

bool GuiGameCollections::OnEntrySelected(InputConfig* config, Input input,
	GameCollectionEntry selectedEntry)
{
	if (config->isMappedTo("a", input) && input.value)
	{
		SetCurrent(selectedEntry.key);
		m_gamelistNeedsReload = true;
		return true;
	}
	else if (config->isMappedTo("x", input) && input.value)
	{
		ShowOptionsMenu(selectedEntry);
	}
	return selectedEntry.optionListComponent->input(config, input);
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
					m_gamelistNeedsReload = true;
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
	std::string duplicateName = name + " Copy";
	std::size_t count = 1;
	while (mGameCollections.GetGameCollection(duplicateName) != nullptr)
	{
		duplicateName = name + " Copy " + std::to_string(++count);
	}
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


