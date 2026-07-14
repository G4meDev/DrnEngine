#include "DrnPCH.h"
#include "Box.h"

namespace Drn
{
	Box& Box::operator+=( const Vector& Other )
	{
		if (bValid)
		{
			Min = Vector( std::min(Min.GetX(), Other.GetX()), std::min(Min.GetY(), Other.GetY()), std::min(Min.GetZ(), Other.GetZ()) );
			Max = Vector( std::max(Max.GetX(), Other.GetX()), std::max(Max.GetY(), Other.GetY()), std::max(Max.GetZ(), Other.GetZ()) );
		}
		else
		{
			Min = Max = Other;
			bValid = true;
		}

		return *this;
	}

	Box& Box::operator+=( const Box& Other )
	{
		if (bValid && Other.bValid)
		{
			Min = Vector( std::min(Min.GetX(), Other.Min.GetX()), std::min(Min.GetY(), Other.Min.GetY()), std::min(Min.GetZ(), Other.Min.GetZ()) );
			Max = Vector( std::max(Max.GetX(), Other.Max.GetX()), std::max(Max.GetY(), Other.Max.GetY()), std::max(Max.GetZ(), Other.Max.GetZ()) );
		}
		else
		{
			*this = Other;
		}

		return *this;
	}

}