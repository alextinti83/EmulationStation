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
	std::shared_ptr<ButtonComponent> addButton(
		const std::string& label, 
		const std::string& helpText,
		const std::function<void()>& callback);

	bool input(InputConfig* config, Input input) override;

	std::vector<HelpPrompt> getHelpPrompts() override;


protected:
	virtual void OnButtonAdded(std::shared_ptr<ButtonComponent> button) { }
	MenuComponent mMenu;
	std::vector< std::function<void()> > mSaveFuncs;
	std::shared_ptr<ButtonComponent> m_backButton;

};