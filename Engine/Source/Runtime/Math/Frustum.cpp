#include "DrnPCH.h"
#include "Frustum.h"

namespace Drn
{
	Frustum::Frustum( const Vector& Location, const Quat& Rotation, float Fov, float AspectRatio, float NearClip, float FarClip )
	{
		XMFLOAT4 R;
		XMStoreFloat4(&R, Rotation.Get());
		const float YSlope = Fov / 45.0f;
		const float XSlope = YSlope * AspectRatio;
		ViewFrustum = DirectX::BoundingFrustum( *Location.Get(), R, XSlope, -XSlope, YSlope, -YSlope, NearClip, FarClip );
	}

	Frustum::Frustum( const ViewInfo& VInfo ) : Frustum(VInfo.Location, VInfo.Rotation, VInfo.FOV, VInfo.AspectRatio, VInfo.NearClipPlane, VInfo.FarClipPlane)
	{}

	//bool Frustum::Contains( const BoxSphereBounds& Bounds ) const
	//{
	//	DirectX::BoundingSphere SphereBound(*Bounds.Origin.Get(), Bounds.SphereRadius);
	//	DirectX::ContainmentType Type = ViewFrustum.Contains(SphereBound);
	//	return Type != DISJOINT;
	//}

	bool Frustum::Contains( const Sphere& Bounds ) const
	{
		DirectX::BoundingSphere SphereBound(*Bounds.Center.Get(), Bounds.Radius);
		DirectX::ContainmentType Type = ViewFrustum.Contains(SphereBound);
		return Type != DISJOINT;
	}

        }  // namespace Drn