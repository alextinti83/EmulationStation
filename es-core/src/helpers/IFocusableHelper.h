#pragma once
#include "IFocusable.h"

namespace focusable
{
	namespace helper
	{
		class Iterator
		{
		public:
			Iterator(FocusPosition pos, const Eigen::Vector2i& cursor, const Eigen::Vector2i& size);
			int end() const { return m_end; }
			Iterator& operator++(); // ++it
			bool operator!=(int it) const;
			bool operator==(int it) const;
			Eigen::Vector2i operator*() const;

		private:
			Eigen::Vector2i GetPos(int i)  const;
			int begin() const { return m_begin; }
			int delta() const { return m_delta; }
			bool IsVertical() const;

		private:
			const FocusPosition pos;
			const Eigen::Vector2i cursor;
			int m_begin;
			int m_end;
			int m_delta;
			int mCursorPos;
		};
	}
}




