#include "views/gamelist/ISimpleGameListView.h"
#include "ThemeData.h"
#include "Window.h"
#include "views/ViewController.h"
#include "Sound.h"
#include "Settings.h"
#include "FileData.h"
#include "SystemData.h"
#include "guis/GuiGameCollections.h"

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) : IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window), m_window(window)
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
		else if (config->isMappedTo("x", input))
		{
			GameCollections* gc = mRoot->getSystem()->GetGameCollections();
			if (gc)
			{
				m_window->pushGui(new GuiGameCollections(m_window, *mRoot->getSystem(), *gc));
				return true;
			}

		}
		else if (config->isMappedTo("y", input))  // Toggle favorites status
		{
			FileData* cursor = getCursor();
			if (cursor->getType() != FOLDER)
			{
				cursor->getSystem()->getIndex()->removeFromIndex(cursor);

				const int cursorIndex = getCursorIndex();
				const int favCount = getFavoritesCount();
				const bool wasFavorite = cursor->isInCurrentGameCollection();
				cursor->SetIsFavorite(!wasFavorite);
				FileChangeType fileChangeType = wasFavorite ? FILE_REMOVED : FILE_ADDED;
				
				cursor->getSystem()->getIndex()->addToIndex(cursor);
				onFileChanged(cursor, fileChangeType);

				if (fileChangeType == FILE_ADDED)
				{
					setCursorIndex(cursorIndex + 1); //keep same file selected
				}
				else
				{
					if (cursorIndex < favCount)
					{
						setCursorIndex(cursorIndex);
					}
				}
				
			}
			return true;
		}
	}

	return IGameListView::input(config, input);
}
