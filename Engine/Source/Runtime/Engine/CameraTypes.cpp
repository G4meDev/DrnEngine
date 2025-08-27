#include "DrnPCH.h"
#include "CameraTypes.h"

#define CLIP_MIN 0.1f
#define CLIP_MAX 10000.0f

namespace Drn
{
	ViewInfo::ViewInfo()
		: Location(Vector::ZeroVector)
		, Rotation(Quat::Identity)
		, ProjectionMode(ECameraProjectionMode::Perspective)
		, FOV(45.0f)
		, OrthoWidth(10.0f)
		, AspectRatio(1.0f)
		, NearClipPlane(CLIP_MIN)
		, FarClipPlane(CLIP_MAX)
	{}

	Matrix ViewInfo::CalculateViewMatrix() const
	{
		XMVECTOR FocusPointOffset = XMVectorSet( 0, 0, 10, 0 );
		FocusPointOffset = XMVector3Rotate(FocusPointOffset, Rotation.Get());
		XMVECTOR m_FocusPoint = XMLoadFloat3(Location.Get()) + FocusPointOffset;

		return XMMatrixLookAtLH( XMLoadFloat3(Location.Get()), m_FocusPoint, XMLoadFloat3(Vector::UpVector.Get()));
	}

	Matrix ViewInfo::CalculateProjectionMatrix() const
	{
		return XMMatrixPerspectiveFovLH( XMConvertToRadians( FOV ), AspectRatio, FarClipPlane, NearClipPlane);
	}

}