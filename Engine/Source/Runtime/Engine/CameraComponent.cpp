#include "DrnPCH.h"
#include "CameraComponent.h"

namespace Drn
{
	CameraComponent::CameraComponent()
		: m_Pos(XMVectorSet( 0, 0, -10, 1 ))
		, m_FocusPoint(XMVectorSet( 0, 0, 0, 1 ))
		, m_UpVector(XMVectorSet( 0, 1, 0, 0 ))
		, m_AspectRatio(1.0f)
		, m_FOV(45.0f)
	{
		
	}

	CameraComponent::~CameraComponent()
	{
		
	}

	void CameraComponent::CalculateMatrices( XMMATRIX& InViewMatrix, XMMATRIX& InProjectionMatrix, float AspectRatio)
	{
		InViewMatrix = XMMatrixLookAtLH( m_Pos, m_FocusPoint, m_UpVector);
		InProjectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians( m_FOV ), AspectRatio, 0.1f, 100.0f );
	}
}