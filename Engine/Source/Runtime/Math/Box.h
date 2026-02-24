#pragma once

#include "Vector.h"

namespace Drn
{
	struct Box
	{
	public:

		Box(const Vector& InMin, const Vector& InMax)
			: Min(InMin)
			, Max(InMax)
		{
		}

		Box() : Box(Vector::OneVector * -1, Vector::OneVector)
		{
		}

		static inline Box BuildAABB(const Vector& Origin, const Vector& Extent)
		{
			return Box(Origin - Extent, Origin + Extent);
		}

		inline Vector GetCenter() const { return Vector((Min + Max) * 0.5f); }
		inline Vector GetExtent() const { return (Max - Min) * 0.5f; }

		void GetCenterAndExtents( Vector& Center, Vector& Extents ) const
		{
			Extents = GetExtent();
			Center = Min + Extents;
		}

		Box& operator+=( const Vector& Other );

		Vector Min;
		Vector Max;

	private:
	};
}