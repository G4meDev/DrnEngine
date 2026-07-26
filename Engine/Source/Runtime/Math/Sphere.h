#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Sphere
	{
	public:

		Sphere(const Vector& InCenter, float InRadius) : Center(InCenter), Radius(InRadius) {};
		Sphere() : Sphere(Vector::OneVector, 1) {};

		Vector Center;
		float Radius;

		inline void Init() { Radius = 0.0f; }
		inline bool IsValid() const { return Radius != 0.0f; }

		bool IsInside(const Sphere& Other, float Tolerance = KINDA_SMALL_NUMBER) const;

		Sphere& operator+=(const Sphere& Other);
	};
}