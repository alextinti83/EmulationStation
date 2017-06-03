#pragma  once
#include "GuiComponent.h"
#include "components/MenuComponent.h"

class GuiRetroArchOptions : public GuiComponent
{
public:
	GuiRetroArchOptions(Window* window);
	virtual ~GuiRetroArchOptions();

	inline void addRow(const ComponentListRow& row) { mMenu.addRow(row); };
	inline void addWithLabel(const std::string& label, const std::shared_ptr<GuiComponent>& comp) { mMenu.addWithLabel(label, comp); };

	bool input(InputConfig* config, Input input) override;

	std::vector<HelpPrompt> getHelpPrompts() override;


private:
	MenuComponent mMenu;
	std::vector< std::function<void()> > mSaveFuncs;
};