#pragma once

#include "ForwardTypes.h"
#include "Runtime/GameFramework/WheeledVehiclePawn.h"

namespace Drn
{
	class WheeledVehiclePawn : public Pawn
	{
	public:

		WheeledVehiclePawn();
		virtual ~WheeledVehiclePawn();

		virtual void Serialize(Archive& Ar) override;
		void Tick( float DeltaTime ) override;

		virtual EActorType GetActorType() override { return EActorType::WheeledVehiclePawn; }
		static EActorType GetActorTypeStatic() { return EActorType::WheeledVehiclePawn; }

		void SetupPlayerInputComponent( class InputComponent* PlayerInputComponent ) override;
		void GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const override;

		void OnThrottle( float Value );
		void OnSteer(float Value);

	protected:

		float ThrottleInput;
		float SteerInput;

		std::unique_ptr<class StaticMeshComponent> VehicleMesh;
		std::unique_ptr<class WheeledVehicleMovementComponent> MovementComponent;

		std::unique_ptr<class StaticMeshComponent> VehicleWheels[4];
	};
}