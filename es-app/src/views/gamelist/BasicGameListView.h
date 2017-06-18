#pragma once

#include "views/gamelist/ISimpleGameListView.h"
#include "components/TextListComponent.h"

class BasicGameListView : public ISimpleGameListView
{
public:
	BasicGameListView(Window* window, FileData* root);

	// Called when a FileData* is added, has its metadata changed, or is removed
	virtual void onFileChanged(FileData* file, FileChangeType change);

	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	virtual FileData* getCursor() override;
	virtual void setCursor(FileData* file) override;

	virtual int getCursorIndex() const override { return mList.getSelectedIndex(); }
	virtual void setCursorIndex(int index) override { mList.setCursorIndex(index); }
	virtual uint32_t getHighlightCount() const override { return mHighlightCount; }

	virtual const char* getName() const override { return "basic"; }

	virtual std::vector<HelpPrompt> getHelpPrompts() override;

protected:
	virtual void populateList(const std::vector<FileData*>& files) override;
	virtual void launch(FileData* game) override;
	virtual void remove(FileData* game) override;

	TextListComponent mList;
	uint32_t mHighlightCount = 0;
};
