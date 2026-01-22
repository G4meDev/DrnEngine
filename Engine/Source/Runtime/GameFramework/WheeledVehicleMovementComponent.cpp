#include "DrnPCH.h"
#include "WheeledVehicleMovementComponent.h"

namespace Drn
{
	WheeledVehicleMovementComponent::WheeledVehicleMovementComponent()
		: Component()
		, Mass(1000.0f)
	{
		// TODO: move socket to vehicle collision to avoid getting stuck. for now it's outside cause there is no support for raycast filtering the vehicle body
		Vector SocketOffset = Vector(2.25, 0.98f, 2.5);
		//Vector SocketOffset = Vector(2.25, 1.1f, 2.5);

		FrontLeftWheel = WheelData();
		FrontLeftWheel.bFrontWheel = true;
		FrontLeftWheel.bRightWheel = false;
		FrontLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, 1);

		FrontRightWheel = WheelData();
		FrontRightWheel.bFrontWheel = true;
		FrontRightWheel.bRightWheel = true;
		FrontRightWheel.SocketLocation = SocketOffset * Vector(1, 1, 1);

		RearLeftWheel = WheelData();
		RearLeftWheel.bFrontWheel = false;
		RearLeftWheel.bRightWheel = false;
		RearLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, -1);

		RearRightWheel = WheelData();
		RearRightWheel.bFrontWheel = false;
		RearRightWheel.bRightWheel = true;
		RearRightWheel.SocketLocation = SocketOffset * Vector(1, 1, -1);
	}

	WheeledVehicleMovementComponent::~WheeledVehicleMovementComponent()
	{
		
	}

	void WheeledVehicleMovementComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		const Vector SpringDirection = GetOwningActor()->GetActorUpVector();

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			WheelData& Wheel = Wheels[i];

			Vector WheelSocketLocation = GetOwningActor()->GetActorTransform().TransformPosition(Wheel.SocketLocation);
			//GetWorld()->DrawDebugSphere(WheelWorldLocation, Quat::Identity, Color::Green, Wheels[i].WheelRadius, 32, 0.0f, 0.0f);

			const Vector RayStart = WheelSocketLocation;
			const Vector RayEnd = WheelSocketLocation + SpringDirection * -Wheel.SuspensionRestLength;
			GetWorld()->DrawDebugLine(RayStart, RayEnd, Color::Blue, 0.0f, 0.0f);

			HitResult Result;
			GetWorld()->GetPhysicScene()->RaycastSingle(Result, RayStart, SpringDirection * -1, Wheel.SuspensionRestLength);

			Wheel.bOnGround = Result.HitActor;
			if (Wheel.bOnGround)
			{
				GetWorld()->DrawDebugSphere(Result.Location, Quat::Identity, Color::Red, 0.1f, 32, 0.0f, 0.0f);

				Vector WorldVelocity = (WheelSocketLocation - Wheel.LastLocation) / DeltaTime;
				float Velocity = Vector::DotProduct(SpringDirection, WorldVelocity);

				Wheel.Offset = Wheel.SuspensionRestLength - Vector::Distance(RayStart, Result.Location);
				Wheel.SuspensionForce = (Wheel.Offset * Wheel.SpringStrength) - (Velocity * Wheel.SpringDamper);
			}
			else
			{
				Wheel.Offset = 0;
				Wheel.SuspensionForce = 0;
			}

			Wheel.LastLocation = WheelSocketLocation;
		}
	}

	void WheeledVehicleMovementComponent::Serialize( Archive& Ar )
	{
		Component::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Mass;
		}
		else
		{
			Ar << Mass;
		}
	}

	void WheeledVehicleMovementComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

	}

	void WheeledVehicleMovementComponent::UnRegisterComponent()
	{
		Component::UnRegisterComponent();
		
	}

#if WITH_EDITOR
	void WheeledVehicleMovementComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

		ImGui::InputFloat("Mass", &Mass);
	}
#endif

}  // namespace Drn