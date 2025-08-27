#include "DrnPCH.h"
#include "CameraComponent.h"

namespace Drn
{
	CameraComponent::CameraComponent()
		:SceneComponent()
		, m_FocusPoint(XMVectorSet( 0, 0, 0, 1 ))
		, m_UpVector(XMVectorSet( 0, 1, 0, 0 ))
		, m_AspectRatio(1.0f)
		, m_FOV(45.0f)
		, m_ClipMin(0.1f)
		, m_ClipMax(10000.0f)
	{
		m_Rotation = XMQuaternionIdentity();
	}

	CameraComponent::~CameraComponent()
	{
		
	}

	void CameraComponent::Tick( float DeltaTime )
	{
		SceneComponent::Tick(DeltaTime);


	}

	void CameraComponent::GetCameraView( ViewInfo& Info )
	{
		Info.Location = GetWorldLocation();
		Info.Rotation = GetWorldRotation();

		Info.ProjectionMode = ECameraProjectionMode::Perspective;
		Info.FOV = m_FOV;
		Info.NearClipPlane = m_ClipMin;
		Info.FarClipPlane = m_ClipMax;

		Info.OrthoWidth = 10.0f;
		Info.AspectRatio = 1.0f;
	}
}