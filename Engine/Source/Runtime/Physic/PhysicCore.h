#pragma once

#include "ForwardTypes.h"

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace physx;

namespace Drn
{
	Vector P2Vector(const PxVec3& Vec);
	Vector Pd2Vector(const PxVec3d& Vec);
	PxVec3 Vector2P(const Vector& Vec);
	PxVec3d Vector2Pd(const Vector& Vec);

	Quat P2Quat(const PxQuat& Q);
	PxQuat Quat2P(const Quat& Q);

	Transform P2Transform(const PxTransform& T);
	PxTransform Transform2P(const Transform& T);
}