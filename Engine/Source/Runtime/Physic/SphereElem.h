#pragma once

#include "ForwardTypes.h"
#include "ShapeElem.h"

namespace Drn
{
	class SphereElem : public ShapeElem
	{
	public:

		SphereElem()
			: ShapeElem(EAggCollisionShape::Sphere)
			, Center(Vector::ZeroVector)
			, Radius(1)
		{
		}

		SphereElem(float R, Vector C = Vector::ZeroVector)
			: ShapeElem(EAggCollisionShape::Sphere)
			, Center(C)
			, Radius(R)
		{
		}

		Vector Center;
		float Radius;

	private:

	};
}