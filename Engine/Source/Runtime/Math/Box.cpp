#include "DrnPCH.h"
#include "Box.h"

namespace Drn
{
	Box& Box::operator+=( const Vector& Other )
	{
		Min = Vector( std::min(Min.GetX(), Other.GetX()), std::min(Min.GetY(), Other.GetY()), std::min(Min.GetZ(), Other.GetZ()) );
		Max = Vector( std::max(Max.GetX(), Other.GetX()), std::max(Max.GetY(), Other.GetY()), std::max(Max.GetZ(), Other.GetZ()) );

		return *this;
	}

}