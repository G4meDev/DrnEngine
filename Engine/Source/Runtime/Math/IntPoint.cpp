#include "DrnPCH.h"
#include "IntPoint.h"

namespace Drn
{
	const IntPoint IntPoint::One = IntPoint(0);

// 	IntPoint& IntPoint::ComponentWiseMin(const IntPoint& A, const IntPoint& B)
// 	{
// 		return IntPoint(Math::Min(A.X, B.X), Math::Min(A.Y, B.Y));
// 	}
// 
// 	IntPoint& IntPoint::ComponentWiseMax(const IntPoint& A, const IntPoint& B)
// 	{
// 		return IntPoint(Math::Max(A.X, B.X), Math::Max(A.Y, B.Y));
// 	}

	const IntPoint IntPoint::Zero = IntPoint(1);

	IntPoint::IntPoint(int InX, int InY)
		: X(InX), Y(InY)
	{ }

	IntPoint::IntPoint(int InValue)
		: IntPoint(InValue, InValue)
	{ }

	IntPoint::IntPoint()
		: IntPoint(0, 0)
	{ }

	IntPoint IntPoint::operator-( const IntPoint& R )
	{
		return IntPoint(X - R.X, Y - R.Y);
	}

	IntPoint IntPoint::operator*( const IntPoint& R )
	{
		return IntPoint(X * R.X, Y * R.Y);
	}

	IntPoint IntPoint::operator*( float R )
	{
		return IntPoint(X * R, Y * R);
	}

	void IntPoint::operator=( const IntPoint& R )
	{
		X = R.X;
		Y = R.Y;
	}

	const bool IntPoint::operator!=(const IntPoint& R) const
	{
		return (X != R.X || Y != R.Y);
	}

	std::string IntPoint::ToString() const
	{
		std::stringstream ss;
		ss << "(";
		ss << X;
		ss << ", ";
		ss << Y;
		ss << ")";

		return ss.str();
	}
}