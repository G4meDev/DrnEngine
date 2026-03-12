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

		BoxSphereBounds( const Box& Box )
		{
			Box.GetCenterAndExtents(Origin, BoxExtent);
			SphereRadius = BoxExtent.Length();
		}

		BoxSphereBounds operator+( const BoxSphereBounds& Other ) const;

		BoxSphereBounds TranslateBy(const Vector& Offset);
		BoxSphereBounds TransformBy(const Transform& T);

		inline Sphere GetSphere() const
		{
			return Sphere(Origin, SphereRadius);
		}

	};
}