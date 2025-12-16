#pragma once

#include "Runtime/Math/Vector2.h"

namespace Drn
{
	class Box2D
	{
	public:

		Vector2 Min;
		Vector2 Max;

		bool bIsValid;

	public:

		Box2D()
			: Min(Vector2(-1))
			, Max(Vector2(1))
			, bIsValid(true)
		{}

		Box2D(const Vector2& InMin, const Vector2& InMax)
			: Min(InMin)
			, Max(InMax)
			, bIsValid(true)
		{}


	};
}