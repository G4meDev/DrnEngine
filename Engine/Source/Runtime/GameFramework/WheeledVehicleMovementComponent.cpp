#include "DrnPCH.h"
#include "WheeledVehicleMovementComponent.h"

namespace Drn
{
	WheeledVehicleMovementComponent::WheeledVehicleMovementComponent()
		: Component()
		, Mass(1000.0f)
	{
		Vector SocketOffset = Vector(2.25, 0.8f, 2.5);

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

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Vector WheelWorldLocation = GetOwningActor()->GetActorTransform().TransformPosition(Wheels[i].SocketLocation);
			GetWorld()->DrawDebugSphere(WheelWorldLocation, Quat::Identity, Color::Green, Wheels[i].WheelRadius, 32, 0.0f, 0.0f);
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