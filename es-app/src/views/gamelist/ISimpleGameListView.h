#pragma once

#include "views/gamelist/IGameListView.h"

#include "components/TextComponent.h"
#include "components/ImageComponent.h"

class ISimpleGameListView : public IGameListView
{
public:
	ISimpleGameListView(Window* window, FileData* root);
	virtual ~ISimpleGameListView() {}

	void update(int deltaTime) override;

	// Called when a new file is added, a file is removed, a file's metadata changes, or a file's children are sorted.
	// NOTE: FILE_SORTED is only reported for the topmost FileData, where the sort started.
	//       Since sorts are recursive, that FileData's children probably changed too.
	virtual void onFileChanged(FileData* file, FileChangeType change);
	
	// Called whenever the theme changes.
	virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	virtual FileData* getCursor() = 0;
	virtual void setCursor(FileData*) = 0;
	virtual int getCursorIndex() const = 0;
	virtual void setCursorIndex(int index) = 0;
	virtual uint32_t getHighlightCount() const = 0;

	virtual bool input(InputConfig* config, Input input) override;
	virtual void launch(FileData* game) = 0;

	std::vector<HelpPrompt> getHelpPrompts() override;
protected:
	void ShowAddGameCollectionUI();
	void AddOrRemoveGameFromCollection();
	void ShowQuestion(const std::string& mgs, const std::function<void()>& func, const std::string& backButton);

	virtual void populateList(const std::vector<FileData*>& files) = 0;

	TextComponent mHeaderText;
	ImageComponent mHeaderImage;
	ImageComponent mBackground;

	std::vector<GuiComponent*> mThemeExtras;

	std::stack<FileData*> mCursorStack;
	Window* m_window;

	bool mHeldPressed;
	bool mPressEventConsumed;
	std::chrono::milliseconds mPressTime;
};
