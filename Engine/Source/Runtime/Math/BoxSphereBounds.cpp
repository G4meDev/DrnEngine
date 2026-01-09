#include "DrnPCH.h"
#include "BoxSphereBounds.h"

namespace Drn
{
	void BoxSphereBounds::TranslateBy( const Vector& Offset )
	{
		Origin = Origin + Offset;
	}

}