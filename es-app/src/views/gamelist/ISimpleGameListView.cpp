#include "views/gamelist/ISimpleGameListView.h"
#include "ThemeData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Sound.h"
#include "Settings.h"
#include "FileData.h"
#include "SystemData.h"
#include "guis/GuiGameCollectionsSettings.h"
#include "guis/GuiMsgBox.h"
#include "GameCollections.h"

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) : IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window), m_window(window), mHeldPressed(false), mPressEventConsumed(false)
{
	mHeaderText.setText("Logo Text");
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setAlignment(ALIGN_CENTER);
	mHeaderText.setDefaultZIndex(50);
	
	mHeaderImage.setResize(0, mSize.y() * 0.185f);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);
	mHeaderImage.setDefaultZIndex(50);

	mBackground.setResize(mSize.x(), mSize.y());
	mBackground.setDefaultZIndex(0);

	addChild(&mHeaderText);
	addChild(&mBackground);
}

void ISimpleGameListView::update(int deltaTime)
{
	IGameListView::update(deltaTime);
	if (mHeldPressed)
	{
		mPressTime += std::chrono::milliseconds(deltaTime);
		if (!mPressEventConsumed && mPressTime > std::chrono::milliseconds(500))
		{
			mPressEventConsumed = true;
			GameCollections* gc = mRoot->getSystem()->GetGameCollections();
			if (gc)
			{
				m_window->pushGui(new GuiGameCollectionsSettings(m_window, *mRoot->getSystem(), *gc));
			}
		}
	}

}

void ISimpleGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	using namespace ThemeFlags;
	mBackground.applyTheme(theme, getName(), "background", ALL);
	mHeaderImage.applyTheme(theme, getName(), "logo", ALL);
	mHeaderText.applyTheme(theme, getName(), "logoText", ALL);

	// Remove old theme extras
	for (auto extra : mThemeExtras)
	{
		removeChild(extra);
		delete extra;
	}
	mThemeExtras.clear();

	// Add new theme extras
	mThemeExtras = ThemeData::makeExtras(theme, getName(), mWindow);
	for (auto extra : mThemeExtras)
	{
		addChild(extra);
	}

	if(mHeaderImage.hasImage())
	{
		removeChild(&mHeaderText);
		addChild(&mHeaderImage);
	}else{
		addChild(&mHeaderText);
		removeChild(&mHeaderImage);
	}
}

void ISimpleGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	// we could be tricky here to be efficient;
	// but this shouldn't happen very often so we'll just always repopulate
	FileData* cursor = getCursor();
	if (!cursor->isPlaceHolder()) {
		populateList(cursor->getParent()->getChildrenListToDisplay());
		setCursor(cursor);
	}
	else
	{
		populateList(mRoot->getChildrenListToDisplay());
		setCursor(cursor);
	}
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	if(input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			FileData* cursor = getCursor();
			if(cursor->getType() == GAME)
			{
				Sound::getFromTheme(getTheme(), getName(), "launch")->play();
				launch(cursor);
			}else{
				// it's a folder
				if(cursor->getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor->getChildrenListToDisplay());
				}
			}

			return true;
		}
		else if(config->isMappedTo("b", input))
		{
			if(mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent()->getChildren());
				setCursor(mCursorStack.top());
				mCursorStack.pop();
				Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}else{
				onFocusLost();
				ViewController::get()->goToSystemView(getCursor()->getSystem());
			}

			return true;
		}
		else if(config->isMappedTo("right", input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToNextGameList();
				return true;
			}
		}
		else if(config->isMappedTo("left", input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToPrevGameList();
				return true;
			}
		}
		else if (config->isMappedTo("y", input))
		{
			mPressTime = std::chrono::milliseconds(0);
			mHeldPressed = true;
			mPressEventConsumed = false;
			return true;
		}
	}
	else //release
	{
		if (config->isMappedTo("y", input))  // add/remove to current game collection
		{
			mHeldPressed = false;
			if (!mPressEventConsumed)
			{
				ShowAddGameCollectionUI();
			}
			return true;
		}
	}
	return IGameListView::input(config, input);
}

std::vector<HelpPrompt> ISimpleGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if (Settings::getInstance()->getBool("QuickSystemSelect"))
		prompts.push_back(HelpPrompt("left/right", "system"));
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("y", "game collections"));
	prompts.push_back(HelpPrompt("select", "options"));

	return prompts;
}


void ISimpleGameListView::AddOrRemoveGameFromCollection()
{
	FileData* cursor = getCursor();
	if (cursor->getType() == GAME)
	{
		cursor->getSystem()->getIndex()->removeFromIndex(cursor);

		const int cursorIndex = getCursorIndex();
		const int highlightCount = getHighlightCount();
		const bool wasInActiveGameCollection = cursor->isInActiveGameCollection();
		cursor->AddToActiveGameCollection(!wasInActiveGameCollection);
		FileChangeType fileChangeType = wasInActiveGameCollection ? FILE_REMOVED : FILE_ADDED;

		if (cursor->GetActiveGameCollectionTag() == GameCollection::Tag::Hide)
		{
			fileChangeType = wasInActiveGameCollection ? FILE_ADDED : FILE_REMOVED;
		}

		cursor->getSystem()->getIndex()->addToIndex(cursor);

		onFileChanged(cursor, fileChangeType);//this will repopulate the list..(twice)

		if (fileChangeType == FILE_ADDED)
		{
			if (cursor->GetActiveGameCollectionTag() == GameCollection::Tag::Highlight)
			{
				setCursorIndex(cursorIndex + 1); //keep same file selected
			}
			else
			{
				setCursorIndex(cursorIndex);
			}
		}
		else
		{
			if (cursorIndex < highlightCount)
			{
				setCursorIndex(cursorIndex);
			}
		}

	}
}

void ISimpleGameListView::ShowQuestion(const std::string& mgs, const std::function<void()>& func)
{
	m_window->pushGui(new GuiMsgBox(mWindow, mgs, "YES", func, "NO", nullptr));
}

void ISimpleGameListView::ShowAddGameCollectionUI()
{
	FileData* cursor = getCursor();
	const GameCollections* gc = mRoot->getSystem()->GetGameCollections();
	if (cursor->getType() == GAME && gc)
	{
		const bool isInActiveGameCollection = cursor->isInActiveGameCollection();
		const std::string gameName = cursor->getName();
		const std::string collectionName = gc->GetActiveGameCollection()->GetName();
		std::string msg = isInActiveGameCollection
			? "Remove " + gameName + " from your \"" + collectionName + "\" collection?"
			: "Add " + gameName + " to your \"" + collectionName + "\" collection?";
		ShowQuestion(msg, [ this ] { AddOrRemoveGameFromCollection(); });
	}
}