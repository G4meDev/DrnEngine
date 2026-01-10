#include "DrnPCH.h"
#include "BoxSphereBounds.h"

namespace Drn
{
	void BoxSphereBounds::TranslateBy( const Vector& Offset )
	{
		Origin = Origin + Offset;
	}

	void BoxSphereBounds::TransformBy( const Transform& T )
	{
		Origin = Origin + T.GetLocation();
		SphereRadius *= T.GetScale().GetMaxComponent();
		BoxExtent = BoxExtent * T.GetScale();
	}

}