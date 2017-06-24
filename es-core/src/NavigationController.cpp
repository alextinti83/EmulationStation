#include "NavigationController.h"
#include <iostream>
#include "InputConfig.h"


void NavigationController::HandleNavigation(const InputData& inputData, IFocusable& rootElement)
{
	if (inputData.input.value != 0)
	{
		const InputConfig& config = inputData.config;
		const Input& input = inputData.input;
		if (config.isMappedTo("up", input))
		{
			rootElement.SetFocus(FocusPosition::Bottom, true);
		}
		else if (config.isMappedTo("down", input))
		{
			rootElement.SetFocus(FocusPosition::Top, true);
		}
		else if (config.isMappedTo("left", input))
		{
			rootElement.SetFocus(FocusPosition::RightMost, true);
		}
		else if (config.isMappedTo("right", input))
		{
			rootElement.SetFocus(FocusPosition::LeftMost, true);
		}
	}
}
