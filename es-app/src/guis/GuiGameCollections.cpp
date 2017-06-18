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
	New, Duplicate, Rename, Delete, Save, Reload, AddAll, RemoveAll
};

GuiGameCollections::GuiGameCollections(
	Window* window,
	SystemData& system,
	GameCollections& gameCollections)
	: GuiOptionWindow(window, "GAME COLLECTIONS")
	, mWindow(window)
	, mSystemData(system)
	, mGameCollections(gameCollections)
	, m_gamelistNeedsReload(false)
	, m_options {
		{ GameCollectionOption::New, "New" },
		{ GameCollectionOption::Duplicate, "Duplicate" },
		{ GameCollectionOption::Rename, "Rename" },
		{ GameCollectionOption::Save, "Save" },
		{ GameCollectionOption::Reload, "Reload" },
		{ GameCollectionOption::Delete, "Delete" },
		{ GameCollectionOption::AddAll, "Add all games" },
		{ GameCollectionOption::RemoveAll, "Remove all games" },
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
	m_entries.clear();
	mMenu.ClearRows();

	GameCollection* current = mGameCollections.GetCurrentGameCollection();

	std::string currentName;
	if (current)
	{
		currentName = current->GetName();
		InsertEntry(currentName);
	}
	for (const GameCollections::GameCollectionMap::value_type& collection : mGameCollections.GetGameCollectionMap())
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
		const std::string name = key;
		const std::string gameCount = " " + std::to_string(gc->GetGameCount()) + "  ";

		auto padding = std::make_shared<TextComponent>(mWindow, "  ", Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		entry.textComponent = std::make_shared<TextComponent>(mWindow, name, Font::get(FONT_SIZE_MEDIUM), 0x777777FF);
		entry.switchComponent = std::make_shared<SwitchComponent>(mWindow);
		auto gameCountTextComp = std::make_shared<TextComponent>(mWindow, gameCount, Font::get(FONT_SIZE_MEDIUM), 0xCCCCCCFF);


		entry.switchComponent->setState(false);
		entry.switchComponent->setVisible(false);
		
		const bool resizeWith = true;
		const bool invertWhenSelected = true;

		row.addElement(entry.switchComponent, !resizeWith, invertWhenSelected);
		row.addElement(padding, !resizeWith);
		row.addElement(entry.textComponent, resizeWith);
		row.addElement(gameCountTextComp, !resizeWith);
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
	else if (config->isMappedTo("y", input) && input.value)
	{
		ShowOptionsMenu(selectedEntry);
	}
	return selectedEntry.optionListComponent->input(config, input);
}

std::vector<HelpPrompt> GuiGameCollections::getHelpPrompts()
{
	return {
				{ "a", "Set Current" },
				{ "b", "Back" },
				{ "y", "Options" },
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
		{
			std::size_t count = 0;
			for (FileData* game : mSystemData.getRootFolder()->getChildrenListToDisplay())
			{
				if (game->getType() == GAME)
				{
					mGameCollections.AddToCurrentGameCollection(*game);
					++count;
				}
			}
			ShowMessage(std::to_string(count) + " games added.", [ this ] () { LoadEntries(); });
			m_gamelistNeedsReload = true;
		} break;
		case GameCollectionOption::RemoveAll:
		{
			GameCollection* gc = mGameCollections.GetCurrentGameCollection();
			if (gc) {
				const std::size_t count = gc->GetGameCount();
				gc->ClearAllGames();
				ShowMessage(std::to_string(count) + " games removed.", [ this ] () { LoadEntries(); });
				m_gamelistNeedsReload = true;
			}
			else
			{
				ShowMessage("Error: could not get the current Game Collection.");
			}
		} break;
		default:
			break;
		}
		return true;
	}
	return false;
}

void GuiGameCollections::NewGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu)
{
	const std::size_t count = mGameCollections.GetGameCollectionMap().size();
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
	if (mGameCollections.GetGameCollectionMap().size() <= 1)
	{
		ShowMessage("You must keep at least 1 Game Collection.");
		return;
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


