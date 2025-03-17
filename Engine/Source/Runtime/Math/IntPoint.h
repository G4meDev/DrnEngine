#pragma once

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

	public:
		static const IntPoint Zero;
		static const IntPoint One;
	};
}

