#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/CameraTypes.h"

namespace Drn
{
	class Frustum
	{
	public:
		Frustum() {};
		Frustum(const Vector& Location, const Quat& Rotation, float Fov, float AspectRatio, float NearClip, float FarClip);
		Frustum(const ViewInfo& VInfo);

		//bool Contains(const BoxSphereBounds& Bounds) const;
		bool Contains(const Sphere& Bounds) const;

	private:
		DirectX::BoundingFrustum ViewFrustum;
	};
}