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

	void CameraComponent::CalculateMatrices( XMMATRIX& InViewMatrix, XMMATRIX& InProjectionMatrix, float AspectRatio )
	{
		XMVECTOR CameraPos = XMLoadFloat3( GetWorldLocation().Get() );
		XMVECTOR CamearRot = GetWorldRotation().Get();

		XMVECTOR FocusPointOffset = XMVectorSet( 0, 0, 10, 0 );
		FocusPointOffset = XMVector3Rotate(FocusPointOffset, CamearRot);

		m_FocusPoint = CameraPos + FocusPointOffset;

		InViewMatrix = XMMatrixLookAtLH( CameraPos, m_FocusPoint, m_UpVector);
		//InProjectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians( m_FOV ), AspectRatio, m_ClipMin, m_ClipMax);
		InProjectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians( m_FOV ), AspectRatio, m_ClipMax, m_ClipMin);
	}

	void CameraComponent::CalculateMatrices( Matrix& InViewMatrix, Matrix& InProjectionMatrix, float AspectRatio )
	{
		XMVECTOR CameraPos = XMLoadFloat3( GetWorldLocation().Get() );
		XMVECTOR CamearRot = GetWorldRotation().Get();

		XMVECTOR FocusPointOffset = XMVectorSet( 0, 0, 10, 0 );
		FocusPointOffset = XMVector3Rotate(FocusPointOffset, CamearRot);

		m_FocusPoint = CameraPos + FocusPointOffset;

		InViewMatrix = XMMatrixLookAtLH( CameraPos, m_FocusPoint, m_UpVector);
		InProjectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians( m_FOV ), AspectRatio, m_ClipMax, m_ClipMin);
	}
}