#include "DrnPCH.h"
#include "IntVector.h"

namespace Drn
{
	IntVector IntVector::Zero = IntVector(0);
	IntVector IntVector::One = IntVector(1);

	bool IntVector::operator==( const IntVector& Other ) const
	{
		return X==Other.X && Y==Other.Y && Z==Other.Z;
	}

	bool IntVector::operator!=( const IntVector& Other ) const
	{
		return X!=Other.X || Y!=Other.Y || Z!=Other.Z;
	}

}