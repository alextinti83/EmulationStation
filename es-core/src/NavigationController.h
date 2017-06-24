#pragma once
#include "INavigationElement.h"
#include <vector>

class NavigationController
{
public:
	void HandleNavigation(const InputData& input, INavigationElement& rootElement);
};