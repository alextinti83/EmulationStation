#pragma  once
#include "GuiOptionWindow.h"
#include "components/ComponentList.h"

class SystemData;
class SwitchComponent;
class GuiSettings;
enum class GameCollectionOption;

class GameCollectionEntry
{
public:
	std::string key;
	std::shared_ptr<TextComponent> textComponent;
	std::shared_ptr<SwitchComponent> switchComponent;
};

class GuiGameCollections : public GuiOptionWindow
{
public:
	GuiGameCollections(Window* window, SystemData& mSystemData);
	virtual ~GuiGameCollections();

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
	void RenameGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu);
	void DeleteGameCollection(const GameCollectionEntry selectedEntry, GuiSettings* menu);

	Window* mWindow;
	SystemData& mSystemData;
	std::map<const std::string, GameCollectionEntry> m_entries;
	std::vector<std::pair<GameCollectionOption, std::string>> m_options;

};