#pragma once

#include "ForwardTypes.h"
#include "ShapeElem.h"

using namespace physx;

namespace Drn
{
	class CapsuleElem : public ShapeElem
	{
	public:

		CapsuleElem(const Vector& InCenter, const Quat& InRotation, float InRadius, float InLength)
			: ShapeElem(EAggCollisionShape::Capsule)
			, Center(InCenter)
			, Rotation(InRotation)
			, Radius(InRadius)
			, Length(InLength)
		{
		}

		CapsuleElem() : CapsuleElem(Vector::ZeroVector, Quat::Identity, 1, 1)
		{
		}

		CapsuleElem(Archive& Ar)
		{
			Serialize(Ar);
		}

		virtual void Serialize(Archive& Ar) override;

		Vector Center;

		// TODO: replace with rotator
		Quat Rotation;

		float Radius;
		float Length;

		inline virtual std::shared_ptr<PxGeometry> GetPxGeometery() override
		{
			return std::shared_ptr<PxGeometry>( new PxCapsuleGeometry( Radius, Length / 2 ) );
		}

	private:
	};
}