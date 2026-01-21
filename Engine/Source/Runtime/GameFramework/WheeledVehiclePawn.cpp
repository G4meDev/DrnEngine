#include "DrnPCH.h"
#include "WheeledVehiclePawn.h"
#include "Runtime/GameFramework/WheeledVehicleMovementComponent.h"
#include "Runtime/Components/InputComponent.h"

namespace Drn
{
	WheeledVehiclePawn::WheeledVehiclePawn()
		: Pawn()
		, ThrottleInput(0)
		, SteerInput(0)
	{
		VehicleMesh = std::make_unique<StaticMeshComponent>();
		VehicleMesh->SetComponentLabel( "VehicleMesh" );
		SetRootComponent(VehicleMesh.get());

		AssetHandle<StaticMesh> CubeMesh("Engine\\Content\\BasicShapes\\SM_Cube.drn");
		CubeMesh.Load();
		VehicleMesh->SetMesh(CubeMesh);

		MovementComponent = std::make_unique<WheeledVehicleMovementComponent>();
		MovementComponent->SetComponentLabel( "WheeledVehicleMovementComponent" );
		AddComponent(MovementComponent.get());
	}

	WheeledVehiclePawn::~WheeledVehiclePawn()
	{
		
	}

	void WheeledVehiclePawn::Serialize( Archive& Ar )
	{
		Pawn::Serialize(Ar);

		MovementComponent->Serialize(Ar);
		VehicleMesh->Serialize(Ar);
	}

	void WheeledVehiclePawn::Tick( float DeltaTime )
	{
		Pawn::Tick(DeltaTime);

		//m_MovementComponent->SetMovementInput(m_MovementInput * 2);

		{
			drn_check(VehicleMesh->GetMesh().IsValid());

			Vector ThrottleForce = GetActorForwardVector() *  ThrottleInput * 10.0f;
			Vector SteerTorque = GetActorUpVector() * SteerInput * 30.0f;

			VehicleMesh->GetBodyInstance().AddForce(ThrottleForce, false);
			VehicleMesh->GetBodyInstance().AddTorque(SteerTorque, false);
		}

		ThrottleInput = 0;
		SteerInput = 0;
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
		OutLocation = GetActorLocation() + Vector(0, 28, -20);
		OutRotation = Matrix::MakeFromZ( GetActorLocation() - OutLocation ).Rotation();
	}

	void WheeledVehiclePawn::OnThrottle( float Value )
	{
		ThrottleInput = Value;
	}

	void WheeledVehiclePawn::OnSteer( float Value )
	{
		SteerInput = Value;
	}

        }  // namespace Drn