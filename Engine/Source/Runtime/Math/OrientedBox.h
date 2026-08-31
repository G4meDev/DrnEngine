#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class OrientedBox
	{
	public:
		union
		{
			DirectX::BoundingOrientedBox DirectxBound;
			struct { Vector Center; Vector Extent; Quat Rotation; };
		};

		OrientedBox(const Vector& InCenter, const Vector& InExtent, const Quat& InRotation)
			: Center(InCenter)
			, Extent(InExtent)
			, Rotation(InRotation)
		{}

		OrientedBox() : OrientedBox(Vector::ZeroVector, Vector::ZeroVector, Quat::Identity) {}

		bool Contains( const Sphere& Bounds ) const;
	};
}