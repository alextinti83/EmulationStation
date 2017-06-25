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
			int begin() const { return m_begin; }
			int end() const { return m_end; }

		public:
			int delta() const { return m_delta; }
			Eigen::Vector2i GetPos(int i)  const;
			bool IsVertical() const;

		private:
			const FocusPosition pos;
			const Eigen::Vector2i cursor;
			int m_begin;
			int m_end;
			int m_delta;
		};

	}
}




