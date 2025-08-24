#pragma once

#include "ForwardTypes.h"
#include "ShapeElem.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace physx;

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

		SphereElem(Archive& Ar)
		{
			Serialize(Ar);
		}

		virtual void Serialize(Archive& Ar) override;

		Vector Center;
		float Radius;

		inline virtual std::shared_ptr<PxGeometry> GetPxGeometery( const Vector& Scale ) override
		{
			float ScaledRadius = Radius * Scale.GetMaxComponent();
			return std::shared_ptr<PxGeometry>( new PxSphereGeometry( ScaledRadius) );
		}

	private:

	};
}