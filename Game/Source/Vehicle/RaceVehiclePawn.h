#pragma once

#include "Runtime/GameFramework/Pawn.h"
#include "ForwardTypes.h"

namespace Drn
{
	class RaceVehiclePawn : public Pawn
	{
	public:
		RaceVehiclePawn();
		virtual ~RaceVehiclePawn();

		virtual void Serialize(Archive& Ar) override;
		void Tick( float DeltaTime ) override;

		virtual EActorType GetActorType() override { return static_cast<EActorType>(EGameActorType::RaceVehiclePawn); }
		static EActorType GetActorTypeStatic() { return static_cast<EActorType>(EGameActorType::RaceVehiclePawn); }

		void SetupPlayerInputComponent( class InputComponent* PlayerInputComponent ) override;
		//void GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const override;
		virtual void CalcCamera( struct ViewInfo& OutResult ) override;

		void OnThrottle( float Value );
		void OnSteer(float Value);

		inline StaticMeshComponent* GetVehicleBody() { return VehicleMesh.get(); }
		inline StaticMeshComponent* GetVehicleWheel(int32 Index) { return VehicleWheels[Index].get(); }

	protected:

		std::unique_ptr<class StaticMeshComponent> VehicleMesh;
		std::unique_ptr<class RaceVehicleMovementComponent> MovementComponent;

		std::unique_ptr<class StaticMeshComponent> VehicleWheels[4];

		std::shared_ptr<class SpringArmComponent> m_SpringArm;
		std::shared_ptr<class CameraComponent> m_Camera;

	};
}