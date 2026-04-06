#include "DrnPCH.h"
#include "WheeledVehiclePawn.h"
#include "Runtime/GameFramework/WheeledVehicleMovementComponent.h"
#include "Runtime/Components/InputComponent.h"

namespace Drn
{
	WheeledVehiclePawn::WheeledVehiclePawn()
		: Pawn()
	{
		VehicleMesh = std::make_unique<StaticMeshComponent>();
		VehicleMesh->SetComponentLabel( "VehicleBody" );
		VehicleMesh->SetStatic(false);

		SceneComponent* OldRoot = GetRoot();
		SetRootComponent(VehicleMesh.get());
		OldRoot->DestroyComponent();

		AssetHandle<StaticMesh> DefaultVehicleBody("Engine\\Content\\DefaultVehicle\\SM_DefaultVehicle_Body.drn");
		DefaultVehicleBody.Load();
		VehicleMesh->SetMesh(DefaultVehicleBody);

		AssetHandle<StaticMesh> DefaultVehicleWheel("Engine\\Content\\DefaultVehicle\\SM_DefaultVehicle_Wheel.drn");
		DefaultVehicleWheel.Load();

		for (int32 i = 0; i < 4; i++)
		{
			VehicleWheels[i] = std::make_unique<StaticMeshComponent>();
			VehicleWheels[i]->SetStatic(false);
			VehicleWheels[i]->SetComponentLabel( "VehicleWheel_" + std::to_string(i) );
			VehicleMesh->AttachSceneComponent(VehicleWheels[i].get());
			SetRootComponent(VehicleMesh.get());
			VehicleWheels[i]->SetMesh(DefaultVehicleWheel);
		}

		MovementComponent = std::make_unique<WheeledVehicleMovementComponent>();
		MovementComponent->SetComponentLabel( "WheeledVehicleMovementComponent" );
		AddComponent(MovementComponent.get());
		//MovementComponent->SetOwningVehicle(this);
		MovementComponent->SetVehicleBody(VehicleMesh.get());
	}

	WheeledVehiclePawn::~WheeledVehiclePawn()
	{
		
	}

	void WheeledVehiclePawn::Serialize( Archive& Ar )
	{
		Pawn::Serialize(Ar);

		MovementComponent->Serialize(Ar);
		VehicleMesh->Serialize(Ar);

		for (int32 i = 0; i < 4; i++)
		{
			VehicleWheels[i]->Serialize(Ar);
		}
	}

	void WheeledVehiclePawn::Tick( float DeltaTime )
	{
		Pawn::Tick(DeltaTime);

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Transform WheelWorldTransform = MovementComponent->GetWheelWorldTransform(i);
			VehicleWheels[i]->SetWorldLocationAndRotation(WheelWorldTransform.GetLocation(), WheelWorldTransform.GetRotation());
		}
	}

	void WheeledVehiclePawn::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &WheeledVehiclePawn::OnThrottle);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);
		
		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &WheeledVehiclePawn::OnSteer);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);
	}

	void WheeledVehiclePawn::GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const
	{
		OutLocation = GetActorLocation() + Vector(0, 28, -20) * 3;
		OutRotation = Matrix::MakeFromZ( GetActorLocation() - OutLocation ).Rotation();
	}

	void WheeledVehiclePawn::OnThrottle( float Value )
	{
		MovementComponent->SetThrottleInput(Value);
	}

	void WheeledVehiclePawn::OnSteer( float Value )
	{
		MovementComponent->SetSteerInput(Value);
	}

        }  // namespace Drn