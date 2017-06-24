#pragma once
#include <Eigen/Dense>

struct Input;
class InputConfig;

enum class FocusPosition { Top, Bottom, LeftMost, RightMost};

namespace focus
{
	struct iterator
	{
		iterator(FocusPosition pos, const Eigen::Vector2i& cursor, const Eigen::Vector2i& size)
			: pos(pos), cursor(cursor)
		{
			if (IsVertical())
			{
				start = pos == FocusPosition::Top ? 0 : size.y() - 1;
				end = pos == FocusPosition::Top ? size.y() : -1;
				delta = pos == FocusPosition::Top ? 1 : -1;
			}
			else
			{
				start = pos == FocusPosition::LeftMost ? 0 : size.x() - 1;
				end = pos == FocusPosition::LeftMost ? size.x() : -1;
				delta = pos == FocusPosition::LeftMost ? 1 : -1;
			}
		}
		int start;
		int end;
		int delta;
		bool IsVertical() const
		{
			return pos == FocusPosition::Top || pos == FocusPosition::Bottom;
		}
		Eigen::Vector2i GetPos(int i)  const
		{
			if (IsVertical())
			{
				return Eigen::Vector2i(cursor.x(), i);
			}
			else
			{
				return Eigen::Vector2i(i, cursor.y());
			}
		}
	private:
		const FocusPosition pos;
		const Eigen::Vector2i cursor;
	};
}

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