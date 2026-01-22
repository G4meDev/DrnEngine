#include "DrnPCH.h"
#include "WheeledVehicleMovementComponent.h"

namespace Drn
{
	WheeledVehicleMovementComponent::WheeledVehicleMovementComponent()
		: Component()
		, OwningVehicle(nullptr)
		, SteerInput(0.0f)
		, ThrottleInput(0.0f)
		, Mass(1000.0f)
	{
		// TODO: move socket to vehicle collision to avoid getting stuck. for now it's outside cause there is no support for raycast filtering the vehicle body
		Vector SocketOffset = Vector(2.25, 0.98f, 2.5);
		//Vector SocketOffset = Vector(2.25, 1.1f, 2.5);

		FrontLeftWheel = WheelData();
		FrontLeftWheel.bFrontWheel = true;
		FrontLeftWheel.bRightWheel = false;
		FrontLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, 1);
		FrontLeftWheel.bEffectedByEngine = true;
		FrontLeftWheel.bEffectedBySteer = true;

		FrontRightWheel = WheelData();
		FrontRightWheel.bFrontWheel = true;
		FrontRightWheel.bRightWheel = true;
		FrontRightWheel.SocketLocation = SocketOffset * Vector(1, 1, 1);
		FrontRightWheel.bEffectedByEngine = true;
		FrontRightWheel.bEffectedBySteer = true;

		RearLeftWheel = WheelData();
		RearLeftWheel.bFrontWheel = false;
		RearLeftWheel.bRightWheel = false;
		RearLeftWheel.SocketLocation = SocketOffset * Vector(-1, 1, -1);
		RearLeftWheel.bEffectedByEngine = true;
		RearLeftWheel.bEffectedBySteer = false;

		RearRightWheel = WheelData();
		RearRightWheel.bFrontWheel = false;
		RearRightWheel.bRightWheel = true;
		RearRightWheel.SocketLocation = SocketOffset * Vector(1, 1, -1);
		RearRightWheel.bEffectedByEngine = true;
		RearRightWheel.bEffectedBySteer = false;
	}

	WheeledVehicleMovementComponent::~WheeledVehicleMovementComponent()
	{
		
	}

	void WheeledVehicleMovementComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		drn_check(OwningVehicle);

		const bool bValidBody = OwningVehicle->GetVehicleBody()->GetMesh().IsValid();
		const Vector SpringDirection = GetOwningActor()->GetActorUpVector();
		const float BodyMass = bValidBody ? OwningVehicle->GetVehicleBody()->GetBodyInstance().GetMass() : 1.0f;

		if (bValidBody)
		{
			GetWorld()->DrawDebugSphere(OwningVehicle->GetVehicleBody()->GetBodyInstance().GetCenterOfMass(), Quat::Identity, Color::Green, 0.2f, 32, 0.0f, 0.0f);
		}

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			WheelData& Wheel = Wheels[i];

			Vector WheelSocketLocation = GetOwningActor()->GetActorTransform().TransformPosition(Wheel.SocketLocation);
			//GetWorld()->DrawDebugSphere(WheelWorldLocation, Quat::Identity, Color::Green, Wheels[i].WheelRadius, 32, 0.0f, 0.0f);
			Vector ForceTarget =  GetOwningActor()->GetActorTransform().TransformPosition(Wheel.SocketLocation + Vector(0.0, Wheel.ForceHeightOffset, 0.0f));

			Vector Displacement = WheelSocketLocation - Wheel.LastLocation;

			Quat WheelRotation = GetOwningActor()->GetActorRotation();
			if (Wheel.bEffectedBySteer)
			{
				WheelRotation = WheelRotation * Quat(Vector::UpVector, SteerInput * Math::DegreesToRadians(Wheel.SteerAngle));
			}

			const Vector RayStart = WheelSocketLocation;
			const Vector RayEnd = WheelSocketLocation + SpringDirection * -Wheel.SuspensionRestLength;
			GetWorld()->DrawDebugLine(RayStart, RayEnd, Color::Blue, 0.0f, 0.0f);

			HitResult Result;
			GetWorld()->GetPhysicScene()->RaycastSingle(Result, RayStart, SpringDirection * -1, Wheel.SuspensionRestLength);

			Wheel.bOnGround = Result.HitActor;
			if (bValidBody && Wheel.bOnGround)
			{
				GetWorld()->DrawDebugSphere(Result.Location, Quat::Identity, Color::Red, 0.1f, 32, 0.0f, 0.0f);

				Vector WorldVelocity = Displacement / DeltaTime;
				float Velocity = Vector::DotProduct(SpringDirection, WorldVelocity);

				Wheel.Offset = Wheel.SuspensionRestLength - Vector::Distance(RayStart, Result.Location);
				float SuspensionForce = (Wheel.Offset * Wheel.SpringStrength) - (Velocity * Wheel.SpringDamper);

				//OwningVehicle->GetVehicleBody()->GetBodyInstance().AddForceAtPosition(SpringDirection * SuspensionForce, Wheel.LastLocation, false);
				OwningVehicle->GetVehicleBody()->GetBodyInstance().AddForceAtPosition(SpringDirection * SuspensionForce, ForceTarget, false);

				if (Wheel.bEffectedByEngine)
				{
					//GetWorld()->DrawDebugLine(WheelSocketLocation, WheelSocketLocation + WheelRotation.GetAxisZ(), Color::Blue, 0.0f, 0.0f);
					float AvaliableTorque = 1000.0f * ThrottleInput;
					Vector ThrottleForce = WheelRotation.GetAxisZ() * AvaliableTorque;

					GetWorld()->DrawDebugSphere(ForceTarget, Quat::Identity, Color::Green, 0.2f, 32, 0.0f, 0.0f);
					OwningVehicle->GetVehicleBody()->GetBodyInstance().AddForceAtPosition(ThrottleForce, ForceTarget, false);
				}

				Vector SteerDirection = WheelRotation.GetAxisX();
				//GetWorld()->DrawDebugLine(WheelSocketLocation, WheelSocketLocation + SteerDirection, Color::Blue, 0.0f, 0.0f);
				float SteerVelocity = Vector::DotProduct(SteerDirection, WorldVelocity);
				float SteerRatio = SteerVelocity == 0 ? 0 : SteerVelocity / WorldVelocity.Length();
				SteerRatio = std::clamp(std::abs(SteerRatio), 0.0f, 1.0f);
				//float Traction = 1 - SteerRatio;
				float Traction = 1.0f;
				
				//std::cout << SteerRatio << "\n";

				float DesireVeloctyChange = -SteerVelocity * BodyMass * Traction / DeltaTime;
				Vector TractionForce = SteerDirection * DesireVeloctyChange * 0.25f;
				OwningVehicle->GetVehicleBody()->GetBodyInstance().AddForceAtPosition(TractionForce, ForceTarget, false);
			}
			else
			{
				Wheel.Offset = 0;
			}

			Wheel.LastLocation = WheelSocketLocation;

			const Vector WheelTransform = Wheel.LastLocation + SpringDirection * -(Wheel.SuspensionRestLength - Wheel.Offset - Wheel.WheelRadius);
			OwningVehicle->GetVehicleWheel(i)->SetWorldLocation(WheelTransform);

			float ForwardDisplacement = Vector::DotProduct(WheelRotation.GetAxisZ(), Displacement);
			Wheel.RotationAngle += ForwardDisplacement / Wheel.WheelRadius;
			Wheel.RotationAngle = std::fmod(Wheel.RotationAngle, XM_2PI);
			if (Wheel.RotationAngle < 0)
			{
				Wheel.RotationAngle += XM_2PI;
			}
			WheelRotation = WheelRotation * Quat(Vector::RightVector, Wheel.RotationAngle);

			OwningVehicle->GetVehicleWheel(i)->SetWorldRotation(WheelRotation);
		}

		ThrottleInput = 0;
		SteerInput = 0;
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