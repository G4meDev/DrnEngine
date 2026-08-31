#include "DrnPCH.h"
#include "OrientedBox.h"

namespace Drn
{
	bool OrientedBox::Contains( const Sphere& Bounds ) const
	{
		return DirectxBound.Contains(Bounds.DirectxBound) != DISJOINT;
	}

}