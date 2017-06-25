#pragma once
#include "IFocusable.h"
#include <vector>

class NavigationController
{
public:
	void HandleNavigation(const InputData& input, IFocusable& rootElement);
};