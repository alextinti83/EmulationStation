#pragma once
#include <Eigen/Dense>

struct Input;
class InputConfig;

class InputData
{
public:
	const InputConfig& config;
	const Input& input;
};

enum class FocusPosition { Top, Bottom, LeftMost, RightMost };

class IFocusable
{
public:
	virtual ~IFocusable() { }
	virtual bool SetFocus(FocusPosition position, bool enableFocus) = 0;
};