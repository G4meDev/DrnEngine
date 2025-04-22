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

		Vector Min;
		Vector Max;

	private:
	};
}