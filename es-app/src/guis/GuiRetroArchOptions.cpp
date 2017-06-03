#include "guis/GuiRetroArchOptions.h"
#include "Window.h"
#include "Settings.h"
#include "views/ViewController.h"

GuiRetroArchOptions::GuiRetroArchOptions(Window* window) 
	: GuiComponent(window)
	, mMenu(window, "RetroArch Configuration")
{

	addChild(&mMenu);

	mMenu.addButton("BACK", "go back", [ this ] { delete this; });

	setSize(( float ) Renderer::getScreenWidth(), ( float ) Renderer::getScreenHeight());
	mMenu.setPosition(( mSize.x() - mMenu.getSize().x() ) / 2, Renderer::getScreenHeight() * 0.15f);
}

GuiRetroArchOptions::~GuiRetroArchOptions()
{

}

bool GuiRetroArchOptions::input(InputConfig* config, Input input)
{
	if (config->isMappedTo("b", input) && input.value != 0)
	{
		delete this;
		return true;
	}

	if (config->isMappedTo("start", input) && 
		input.value != 0)
	{
		// close everything
		Window* window = mWindow;
		while (window->peekGui() && window->peekGui() != ViewController::get())
			delete window->peekGui();
		return true;
	}

	return GuiComponent::input(config, input);
}

std::vector<HelpPrompt> GuiRetroArchOptions::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();

	prompts.push_back(HelpPrompt("b", "back"));
	prompts.push_back(HelpPrompt("select", "close"));

	return prompts;
}