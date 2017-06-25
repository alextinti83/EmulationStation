#pragma once
#include "IFocusableHelper.h"

namespace focusable
{
	namespace helper
	{
		Iterator::Iterator(FocusPosition pos, const Eigen::Vector2i& cursor, const Eigen::Vector2i& size)
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
			mCursorPos = m_begin;
		}

		Iterator& Iterator::operator++()
		{
			mCursorPos += delta();
			return *this;
		}

		Eigen::Vector2i Iterator::operator*() const
		{
			return GetPos(mCursorPos);
		}

		bool Iterator::operator!=(int it) const
		{
			return !(*this == it);
		}

		bool Iterator::operator==(int it) const
		{
			return mCursorPos == it;
		}

		Eigen::Vector2i Iterator::GetPos(int i)  const
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

		bool Iterator::IsVertical() const
		{
			return pos == FocusPosition::Top || pos == FocusPosition::Bottom;
		}
	}
}




