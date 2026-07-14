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
			, bValid(true)
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

		inline void Init()
		{
			Min = Max = Vector::ZeroVector;
			bValid = false;
		}

		Box& operator+=( const Vector& Other );
		Box& operator+=( const Box& Other );

		Vector Min;
		Vector Max;

		bool bValid;

	private:
	};
}