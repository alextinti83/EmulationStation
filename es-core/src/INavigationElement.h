#pragma once
#include <Eigen/Dense>

struct Input;
class InputConfig;

enum class FocusPosition { Top, Bottom, LeftMost, RightMost};
using ResetFocusCallback =std::function<bool(FocusPosition)>;

class InputData
{
public:
	const InputConfig& config;
	const Input& input;
};


class INavigationElement
{
public:
	virtual ~INavigationElement() { }
	virtual bool SetFocusPosition(FocusPosition position, bool focus) = 0;
};