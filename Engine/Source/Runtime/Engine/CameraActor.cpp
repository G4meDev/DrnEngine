#include "DrnPCH.h"
#include "CameraActor.h"

namespace Drn
{
	CameraActor::CameraActor()
		: Actor()
	{
		m_CameraComponenet = std::make_unique<CameraComponent>();
		SetRootComponent( m_CameraComponenet.get() );

#if WITH_EDITOR
		m_CameraComponenet->SetComponentLabel( "Camera" );
#endif
	}

	CameraActor::~CameraActor()
	{
		
	}

	void CameraActor::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);

	}

#if WITH_EDITOR
	void CameraActor::ApplyViewportInput( const ViewportCameraInputHandler& CameraInput,
		float CameraMovementSpeed, float CameraRotationSpeed )
	{
		XMVECTOR CamPos = XMLoadFloat3( GetActorLocation().Get() );
		XMVECTOR CamRot = GetActorRotation().Get();

		XMVECTOR Displacement = XMVector3Rotate( CameraInput.m_Displacement * CameraMovementSpeed, CamRot );
		SetActorLocation( CamPos + Displacement );

		XMVECTOR Axis_Y = XMVectorSet( 0, 1, 0, 0 );
		XMVECTOR Axis_X = XMVectorSet( 1, 0, 0, 0 );
		Axis_X = XMVector3Rotate(Axis_X, CamRot);

		XMVECTOR Rot_Offset_X = XMQuaternionRotationAxis( Axis_Y, CameraInput.m_MouseDelta.X * CameraRotationSpeed );
		XMVECTOR Rot_Offset_Y = XMQuaternionRotationAxis( Axis_X, CameraInput.m_MouseDelta.Y * CameraRotationSpeed );

		SetActorRotation( XMQuaternionMultiply( CamRot, Rot_Offset_Y ) );
		SetActorRotation( XMQuaternionMultiply( GetActorRotation().Get(), Rot_Offset_X ) );
	}
#endif

}