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

namespace focusable
{
	namespace helpers
	{
		class Iterator
		{
		public:
			Iterator(FocusPosition pos, const Eigen::Vector2i& cursor, const Eigen::Vector2i& size)
				: pos(pos), cursor(cursor)
			{
				if (IsVertical())
				{
					m_begin = pos == FocusPosition::Top ? 0 : size.y() - 1;
					m_end = pos == FocusPosition::Top ? size.y() : -1;
					m_delta = pos == FocusPosition::Top ? 1 : -1;
				}
				else
				{
					m_begin = pos == FocusPosition::LeftMost ? 0 : size.x() - 1;
					m_end = pos == FocusPosition::LeftMost ? size.x() : -1;
					m_delta = pos == FocusPosition::LeftMost ? 1 : -1;
				}
			}

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

			int begin() const { return m_begin; }
			int end() const { return m_end; }
			int delta() const { return m_delta; }

		private:
			const FocusPosition pos;
			const Eigen::Vector2i cursor;
			int m_begin;
			int m_end;
			int m_delta;
		};
	}
}




