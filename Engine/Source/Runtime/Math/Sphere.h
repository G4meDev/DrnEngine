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

	};
}