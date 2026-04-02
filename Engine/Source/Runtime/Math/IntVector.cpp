#include "DrnPCH.h"
#include "IntVector.h"

namespace Drn
{
	IntVector IntVector::Zero = IntVector( 0 );
	IntVector IntVector::One = IntVector(1);

	bool IntVector::operator==( const IntVector& Other ) const
	{
		return X==Other.X && Y==Other.Y && Z==Other.Z;
	}

	bool IntVector::operator!=( const IntVector& Other ) const
	{
		return X!=Other.X || Y!=Other.Y || Z!=Other.Z;
	}

	IntVector IntVector::DivideAndRoundUp( IntVector lhs, int32 Divisor )
	{
		return IntVector(Math::DivideAndRoundUp(lhs.X, Divisor), Math::DivideAndRoundUp(lhs.Y, Divisor), Math::DivideAndRoundUp(lhs.Z, Divisor));
	}
}