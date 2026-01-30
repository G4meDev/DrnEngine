#include "GamePCH.h"
#include "RaceVehiclePawn.h"

#include "Runtime/Components/InputComponent.h"
#include "Vehicle/RaceVehicleMovementComponent.h"

#include "Runtime/Components/SpringArmComponent.h"
#include "Runtime/Engine/CameraComponent.h"

namespace Drn
{
	RaceVehiclePawn::RaceVehiclePawn()
		: Pawn()
	{
		VehicleMesh = std::make_unique<StaticMeshComponent>();
		VehicleMesh->SetComponentLabel( "VehicleBody" );
		VehicleMesh->SetStatic(false);
		SetRootComponent(VehicleMesh.get());

		AssetHandle<StaticMesh> DefaultVehicleBody("Game\\Content\\Level_2\\Vehicle\\SM_VehicleBody.drn");
		DefaultVehicleBody.Load();
		VehicleMesh->SetMesh(DefaultVehicleBody);

		AssetHandle<StaticMesh> DefaultVehicleWheel("Game\\Content\\Level_2\\Vehicle\\SM_VehicleWheel.drn");
		DefaultVehicleWheel.Load();

		m_SpringArm = std::make_shared<SpringArmComponent>();
		GetRoot()->AttachSceneComponent(m_SpringArm.get());
		m_SpringArm->SetComponentLabel("SpringArm");

		m_Camera = std::make_shared<CameraComponent>();
		m_SpringArm->AttachSceneComponent(m_Camera.get());
		m_Camera->SetComponentLabel("Camera");

		m_SpringArm->SetArmLength(11.0f);
		m_SpringArm->SetRelativeLocationAndRotation(Vector(0.0f, 2.0, 0.0f), Quat(0, XM_PI / 12, 0));
		m_SpringArm->SetEnableLocationLag(true);
		m_SpringArm->SetLocationLagSpeed(20.0f);
		m_SpringArm->SetEnableRotationLag(true);
		m_SpringArm->SetRotationLagSpeed(30.0f);

		for (int32 i = 0; i < 4; i++)
		{
			VehicleWheels[i] = std::make_unique<StaticMeshComponent>();
			VehicleWheels[i]->SetStatic(false);
			VehicleWheels[i]->SetComponentLabel( "VehicleWheel_" + std::to_string(i) );
			VehicleMesh->AttachSceneComponent(VehicleWheels[i].get());
			SetRootComponent(VehicleMesh.get());
			VehicleWheels[i]->SetMesh(DefaultVehicleWheel);
		}

		MovementComponent = std::make_unique<RaceVehicleMovementComponent>();
		MovementComponent->SetComponentLabel( "RaceVehicleMovementComponent" );
		AddComponent(MovementComponent.get());
		//MovementComponent->SetOwningVehicle(this);
		MovementComponent->SetVehicleBody(VehicleMesh.get());
	}

	RaceVehiclePawn::~RaceVehiclePawn()
	{
		
	}

	void RaceVehiclePawn::Serialize( Archive& Ar )
	{
		Pawn::Serialize(Ar);

		MovementComponent->Serialize(Ar);
		VehicleMesh->Serialize(Ar);

		for (int32 i = 0; i < 4; i++)
		{
			VehicleWheels[i]->Serialize(Ar);
		}
	}

	void RaceVehiclePawn::Tick( float DeltaTime )
	{
		Pawn::Tick(DeltaTime);

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Transform WheelWorldTransform = MovementComponent->GetWheelWorldTransform(i);
			VehicleWheels[i]->SetWorldLocationAndRotation(WheelWorldTransform.GetLocation(), WheelWorldTransform.GetRotation());
		}
	}

	void RaceVehiclePawn::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &RaceVehiclePawn::OnThrottle);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);
		
		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &RaceVehiclePawn::OnSteer);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);
	}

	void RaceVehiclePawn::CalcCamera( struct ViewInfo& OutResult )
	{
		m_Camera->GetCameraView( OutResult );
	}

	//void RaceVehiclePawn::GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const
	//{
	//	OutLocation = GetActorLocation() + Vector(0, 28, -20) * 3;
	//	OutRotation = Matrix::MakeFromZ( GetActorLocation() - OutLocation ).Rotation();
	//}

	void RaceVehiclePawn::OnThrottle( float Value )
	{
		MovementComponent->SetThrottleInput(Value);
	}

	void RaceVehiclePawn::OnSteer( float Value )
	{
		MovementComponent->SetSteerInput(Value);
	}

}  // namespace Drn