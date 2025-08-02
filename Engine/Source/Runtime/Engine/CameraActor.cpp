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

#ifndef WITH_EDITOR

		POINT A;
		GetCursorPos(&A);
		IntPoint R(A.x, A.y); 

		IntPoint Delta = R - MousePos;
		MousePos = R;

		bool wDown = GetKeyState( 'W' ) & 0x8000;
		bool aDown = GetKeyState( 'A' ) & 0x8000;
		bool sDown = GetKeyState( 'S' ) & 0x8000;
		bool dDown = GetKeyState( 'D' ) & 0x8000;
		bool eDown = GetKeyState( 'E' ) & 0x8000;
		bool qDown = GetKeyState( 'Q' ) & 0x8000;

		float ForwardDis = wDown - sDown;
		float RightDis   = dDown - aDown;
		float UpDis      = eDown - qDown;

		XMVECTOR D = XMVectorSet( RightDis * DeltaTime, UpDis * DeltaTime, ForwardDis * DeltaTime, 0 );

		XMVECTOR CamPos = XMLoadFloat3( GetActorLocation().Get() );
		XMVECTOR CamRot = GetActorRotation().Get();

		XMVECTOR Displacement = XMVector3Rotate( D * 10.0f, CamRot );
		SetActorLocation( CamPos + Displacement );

		XMVECTOR Axis_Y = XMVectorSet( 0, 1, 0, 0 );
		XMVECTOR Axis_X = XMVectorSet( 1, 0, 0, 0 );
		Axis_X = XMVector3Rotate(Axis_X, CamRot);

		XMVECTOR Rot_Offset_X = XMQuaternionRotationAxis( Axis_Y, Delta.X * 0.01f);
		XMVECTOR Rot_Offset_Y = XMQuaternionRotationAxis( Axis_X, Delta.Y * 0.01f);

		SetActorRotation( XMQuaternionMultiply( CamRot, Rot_Offset_Y ) );
		SetActorRotation( XMQuaternionMultiply( GetActorRotation().Get(), Rot_Offset_X ) );
#endif

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