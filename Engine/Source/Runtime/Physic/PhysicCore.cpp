#include "DrnPCH.h"
#include "PhysicCore.h"

namespace Drn
{
	Vector P2Vector( const PxVec3& Vec )
	{
		return Vector(Vec.x, Vec.y, Vec.z);
	}

	PxVec3 Vector2P( const Vector& Vec )
	{
		return PxVec3(Vec.GetX(), Vec.GetY(), Vec.GetZ());
	}

	Quat P2Quat( const PxQuat& Q )
	{
		return Quat(Q.x, Q.y, Q.z, Q.w);
	}

	PxQuat Quat2P( const Quat& Q )
	{
		return PxQuat(Q.GetX(), Q.GetY(), Q.GetZ(), Q.GetW());
	}

	Transform P2Transform( const PxTransform& T )
	{
		return Transform( P2Vector(T.p), P2Quat(T.q), Vector::OneVector);
	}

	PxTransform Transform2P( const Transform& T )
	{
		return PxTransform(Vector2P(T.GetLocation()), Quat2P(T.GetRotation()));
	}

}