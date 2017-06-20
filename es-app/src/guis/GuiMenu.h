#pragma once

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include <functional>
class StrInputConfig;
class GuiSettings;
class GuiMenu : public GuiComponent
{
public:
	GuiMenu(Window* window);

	bool input(InputConfig* config, Input input) override;
	void onSizeChanged() override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	HelpStyle getHelpStyle() override;

private:
	void createConfigInput();
	void clearLoadedInput();
	void addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func);
	void addTemperatureEntry(GuiSettings* s);
	void addSystemsEntry(GuiSettings* s);
	void openScreensaverOptions();
	MenuComponent mMenu;
	TextComponent mVersion;
	std::vector<StrInputConfig*> mLoadedInput; // used to keep information about loaded devices in case there are unpluged between device window load and save

};
