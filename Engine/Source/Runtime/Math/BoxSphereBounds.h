#pragma once

#include "Runtime/Math/Vector.h"

namespace Drn
{
	struct BoxSphereBounds
	{
		Vector Origin;
		Vector BoxExtent;
		float SphereRadius;

		BoxSphereBounds()
			: Origin(Vector::ZeroVector)
			, BoxExtent(Vector::OneVector)
			, SphereRadius(1)
		{}

		BoxSphereBounds(const Vector& InOrigin, const Vector& InBoxExtent, float InSphereRadius)
			: Origin(InOrigin)
			, BoxExtent(InBoxExtent)
			, SphereRadius(InSphereRadius)
		{}

	};
}