#pragma  once
#include "guis/GuiOptionWindow.h"
#include "components/ComponentList.h"
#include "components/OptionListComponent.h"
#include "GameCollection.h"

class SystemData;
class GameCollections;
class SwitchComponent;
class GuiSettings;
enum class GameCollectionOption;

class GameCollectionEntry
{
public:
	std::string key;
	std::shared_ptr<TextComponent> textComponent;
	std::shared_ptr<SwitchComponent> switchComponent;
	std::shared_ptr<OptionListComponent<GameCollection::Tag> > optionListComponent;
};

class GuiGameCollectionsSettings : public GuiOptionWindow
{
public:
	GuiGameCollectionsSettings(Window* window, SystemData& system, GameCollections& gameCollections);
	~GuiGameCollectionsSettings() override;

	void LoadEntries();
	void InsertEntry(const std::string& entryName);
	std::vector<HelpPrompt> getHelpPrompts() override;
private:
	bool OnEntrySelected(InputConfig* config, Input input, GameCollectionEntry selectedEntry);
	GameCollectionEntry* GetEntry(const std::string key);
	void SetCurrent(const std::string key);
	void ShowMessage(const std::string& mgs, const std::function<void()>& func1 = nullptr);
	void ShowQuestion(const std::string& mgs, const std::function<void()>& func);

	void ShowOptionsMenu(GameCollectionEntry selectedEntry);
	void InsertRow(GuiSettings& root, const std::string& text, std::function<bool(InputConfig*, Input)> fun);
	bool OnOptionSelected(
		InputConfig* config, 
		Input input, 
		const GameCollectionOption option, 
		const GameCollectionEntry selectedEntry, 
		GuiSettings* menu);
	void NewGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu);
	void DuplicateGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu);
	void RenameGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu);
	void DeleteGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu);

	inline void AddOnCloseFunction(const std::function<void()>& func) { m_onCloseFunctions.push_back(func); };

	Window* mWindow;
	SystemData& mSystemData;
	GameCollections& mGameCollections;
	std::map<const std::string, GameCollectionEntry> m_entries;
	std::vector<std::pair<GameCollectionOption, std::string>> m_options;
	std::vector< std::function<void()> > m_onCloseFunctions;
	bool m_gamelistNeedsReload;
};