#include "views/gamelist/BasicGameListView.h"
#include "views/ViewController.h"
#include "Renderer.h"
#include "Window.h"
#include "ThemeData.h"
#include "SystemData.h"
#include "Settings.h"
#include "FileFilterIndex.h"

BasicGameListView::BasicGameListView(Window* window, FileData* root)
	: ISimpleGameListView(window, root), mList(window)
{
	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);
	mList.setDefaultZIndex(20);
	addChild(&mList);

	populateList(root->getChildrenListToDisplay());
}

void BasicGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	ISimpleGameListView::onThemeChanged(theme);
	using namespace ThemeFlags;
	mList.applyTheme(theme, getName(), "gamelist", ALL);

	sortChildren();
}

void BasicGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	if(change == FILE_METADATA_CHANGED)
	{
		// might switch to a detailed view
		ViewController::get()->reloadGameListView(this);
		return;
	}

	ISimpleGameListView::onFileChanged(file, change);
}

void BasicGameListView::populateList(const std::vector<FileData*>& files)
{
	mList.clear();
	std::size_t hiddenCount = 0;
	if (files.size() > 0)
	{
		mHeaderText.setText(files.at(0)->getSystem()->getFullName());

		std::vector<FileData*> games, folders, highlights;

		for ( FileData* filedata : files )
		{
			if ( filedata->getType() == FOLDER )
			{
				folders.push_back(filedata);
			}
			else
			{
				if (filedata->isHidden())
				{
					hiddenCount++;
				}
				else
				{
					games.push_back(filedata);
				}
				if (filedata->isHighlighted())
				{
					highlights.push_back(filedata);
				}
			}
		}
		mHighlightCount = highlights.size();
		FileFilterIndex* idx = this->mRoot->getSystem()->getIndex();
		if (!idx->isFilteredByType(FAVORITES_FILTER))
		{
			for (FileData* filedata : highlights)
			{
				mList.add("  " + filedata->getName(), filedata, 0, 2);
			}
		}
		for ( FileData* filedata : folders )
		{
			mList.add("[ " + filedata->getName() + " ]", filedata, 1, 0);
		}
		for ( FileData* filedata : games )
		{
			mList.add("  " + filedata->getName(), filedata, 0, 0);
		}

	}
	if (mList.size() == 0)
	{
		// empty list - add a placeholder
		FileData* placeholder = new FileData(PLACEHOLDER, "<No Results Found for Current Filter Criteria>", this->mRoot->getSystem());
		mList.add(placeholder->getName(), placeholder, (placeholder->getType() == PLACEHOLDER), 0);
		if (hiddenCount > 0)
		{
			const std::string text = "<" + std::to_string(hiddenCount) + " hidden games>";
			FileData* placeholder = new FileData(PLACEHOLDER, text, this->mRoot->getSystem());
			mList.add(placeholder->getName(), placeholder, ( placeholder->getType() == PLACEHOLDER ), 0);
		}
	}
}


FileData* BasicGameListView::getCursor()
{
	return mList.getSelected();
}

void BasicGameListView::setCursor(FileData* cursor)
{
	if (cursor->isPlaceHolder())
		return;
	if(!mList.setCursor(cursor))
	{
		populateList(cursor->getParent()->getChildrenListToDisplay());
		mList.setCursor(cursor);

		// update our cursor stack in case our cursor just got set to some folder we weren't in before
		if(mCursorStack.empty() || mCursorStack.top() != cursor->getParent())
		{
			std::stack<FileData*> tmp;
			FileData* ptr = cursor->getParent();
			while(ptr && ptr != mRoot)
			{
				tmp.push(ptr);
				ptr = ptr->getParent();
			}

			// flip the stack and put it in mCursorStack
			mCursorStack = std::stack<FileData*>();
			while(!tmp.empty())
			{
				mCursorStack.push(tmp.top());
				tmp.pop();
			}
		}
	}
}

void BasicGameListView::launch(FileData* game)
{
	ViewController::get()->launch(game);
}

void BasicGameListView::remove(FileData *game)
{
	boost::filesystem::remove(game->getPath());  // actually delete the file on the filesystem
	if (getCursor() == game)                     // Select next element in list, or prev if none
	{
		std::vector<FileData*> siblings = game->getParent()->getChildren();
		auto gameIter = std::find(siblings.begin(), siblings.end(), game);
		auto gamePos = std::distance(siblings.begin(), gameIter);
		if (gameIter != siblings.end())
		{
			if ((gamePos + 1) < siblings.size())
			{
				setCursor(siblings.at(gamePos + 1));
			} else if ((gamePos - 1) > 0) {
				setCursor(siblings.at(gamePos - 1));
			}
		}
	}
	delete game;                                 // remove before repopulating (removes from parent)
	onFileChanged(game, FILE_REMOVED);           // update the view, with game removed
}

std::vector<HelpPrompt> BasicGameListView::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;

	if(Settings::getInstance()->getBool("QuickSystemSelect"))
		prompts.push_back(HelpPrompt("left/right", "system"));
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "launch"));
	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("y", "game collect. options")); 
	prompts.push_back(HelpPrompt("x", "+/- game collect.")); 
	prompts.push_back(HelpPrompt("select", "options"));
	return prompts;
}
