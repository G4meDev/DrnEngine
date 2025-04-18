#pragma once

#include "ForwardTypes.h"
#include "ShapeElem.h"

namespace Drn
{
	class BoxElem : public ShapeElem
	{
	public:

		BoxElem(const Vector& InCenter, const Quat& InRotation, const Vector& InScale)
			: ShapeElem(EAggCollisionShape::Box)
			, Center(InCenter)
			, Rotation(InRotation)
			, Scale(InScale)
		{
		}

		BoxElem() : BoxElem(Vector::ZeroVector, Quat::Identity, Vector::OneVector)
		{
		}

		Vector Center;

		// TODO: use rotator
		Quat Rotation;

		Vector Scale;

		inline virtual std::shared_ptr<PxGeometry> GetPxGeometery() override
		{
			return std::shared_ptr<PxGeometry>( new PxBoxGeometry( Vector2P(Scale) ) );
		}

	private:
	};
}