#include "NavigationController.h"
#include <iostream>
#include "InputConfig.h"
/*
	Some notes:
	The navigation is already managed locally inside each GuiComponent instance,
	but here is where I would handle it; at a global level.
	Most likely with a navigation delegate visiting the IFocusable/GuiComponent composite hierarchy blah blah..
	I believe it won't happen any time soon due my to lack of time.

	With a local navigation management, looping through menu entries is not straightforward unfortunately,
	so, as workaround to it, this HandleNavigation is now just a fallback 
	aiming to focus the entry at the location opposite
	to the direction received as - unhandled - input.
*/
void NavigationController::HandleNavigation(const InputData& inputData, IFocusable& rootElement)
{
	if (inputData.input.value != 0)
	{
		const InputConfig& config = inputData.config;
		const Input& input = inputData.input;
		const bool enableFocus = true;
		if (config.isMappedTo("up", input))
		{
			rootElement.UpdateFocus(FocusPosition::Bottom, enableFocus);
		}
		else if (config.isMappedTo("down", input))
		{
			rootElement.UpdateFocus(FocusPosition::Top, enableFocus);
		}
		else if (config.isMappedTo("left", input))
		{
			rootElement.UpdateFocus(FocusPosition::RightMost, enableFocus);
		}
		else if (config.isMappedTo("right", input))
		{
			rootElement.UpdateFocus(FocusPosition::LeftMost, enableFocus);
		}
	}
}
