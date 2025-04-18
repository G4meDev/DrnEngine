#pragma once

#include "ForwardTypes.h"
#include "ShapeElem.h"

namespace Drn
{
	class BoxElem : public ShapeElem
	{
	public:

		BoxElem(const Vector& InCenter, const Quat& InRotation, const Vector& InExtent)
			: ShapeElem(EAggCollisionShape::Box)
			, Center(InCenter)
			, Rotation(InRotation)
			, Extent(InExtent)
		{
		}

		BoxElem() : BoxElem(Vector::ZeroVector, Quat::Identity, Vector::OneVector)
		{
		}

		BoxElem(Archive& Ar)
		{
			Serialize(Ar);
		}

		virtual void Serialize(Archive& Ar) override;

		Vector Center;

		// TODO: use rotator
		Quat Rotation;

		Vector Extent;

		inline virtual std::shared_ptr<PxGeometry> GetPxGeometery() override
		{
			return std::shared_ptr<PxGeometry>( new PxBoxGeometry( Vector2P(Extent) ) );
		}

	private:
	};
}