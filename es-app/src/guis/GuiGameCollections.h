#pragma  once
#include "GuiOptionWindow.h"
#include "components/ComponentList.h"

class SystemData;
class SwitchComponent;

class GameCollectionEntry : public ComponentListRow
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

	void InsertEntry(const std::string& entryName);
private:
	bool OnEntrySelected(InputConfig* config, Input input, GameCollectionEntry* selectedEntry);
	GameCollectionEntry* GetEntry(const std::string key);
	void SetCurrent(const std::string key);

	Window* mWindow;
	SystemData& mSystemData;
	std::map<const std::string, GameCollectionEntry> m_entries;
};