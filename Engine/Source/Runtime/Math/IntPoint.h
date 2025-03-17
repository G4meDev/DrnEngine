#pragma once

#include <string>

namespace Drn
{
	struct IntPoint
	{
	public:
		int X;
		int Y;

		IntPoint(int InX, int InY);
		IntPoint(int InValue);
		IntPoint();

		IntPoint operator*(const IntPoint& R);
		void operator=(const IntPoint& R);
		const bool operator!=(const IntPoint& R) const;

		static const IntPoint Zero;
		static const IntPoint One;

		std::string ToString() const;
	};
}

