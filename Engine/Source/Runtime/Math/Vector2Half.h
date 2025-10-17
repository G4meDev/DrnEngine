#pragma once

#include "Runtime/Math/Float16.h"

namespace Drn
{
	class Vector2Half
	{
	public:
		Float16 X;
		Float16 Y;

		inline Vector2Half() {}
		inline Vector2Half(const Float16& InX, const Float16& InY) : X(InX), Y(InY) {}
		inline Vector2Half(float InX, float InY) : X(InX), Y(InY) {}

		//inline Vector2Half(const Vector2& InVector2) : X(InX), Y(InY) {}
	};
}