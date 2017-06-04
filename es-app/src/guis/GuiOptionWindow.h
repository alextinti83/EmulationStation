#pragma  once
#include "GuiComponent.h"
#include "components/MenuComponent.h"

class GuiOptionWindow : public GuiComponent
{
public:
	GuiOptionWindow(Window* window, const std::string& title);
	virtual ~GuiOptionWindow();

	inline void addRow(const ComponentListRow& row) { mMenu.addRow(row); };
	inline void addWithLabel(const std::string& label, const std::shared_ptr<GuiComponent>& comp) { mMenu.addWithLabel(label, comp); };

	bool input(InputConfig* config, Input input) override;

	std::vector<HelpPrompt> getHelpPrompts() override;


protected:
	MenuComponent mMenu;
	std::vector< std::function<void()> > mSaveFuncs;
};