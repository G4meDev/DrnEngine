#include "GamePCH.h"
#include "RaceVehicleMovementComponent.h"

namespace Drn
{
	RaceVehicleMovementComponent::RaceVehicleMovementComponent()
		: WheeledVehicleMovementComponent()
	{
		// TODO: remove
		gPhysXMaterialFrictions[0].friction = 1.0f;
		gPhysXMaterialFrictions[0].material = PhysicManager::Get()->TempMaterial;

		Vector FrontSocketOffset = Vector(2, 0.75f, 2.7);
		Vector RearSocketOffset = Vector(2, 0.75f, 2.6);

		FrontLeftWheel.SocketLocation = FrontSocketOffset * Vector(-1, 1, 1);
		FrontRightWheel.SocketLocation = FrontSocketOffset * Vector(1, 1, 1);
		RearLeftWheel.SocketLocation = RearSocketOffset * Vector(-1, 1, -1);
		RearRightWheel.SocketLocation = RearSocketOffset * Vector(1, 1, -1);

		for (int32 i = 0; i < NUM_WHEELS; i++)
		{
			Wheels[i].Radius = 0.8f;
			Wheels[i].HalfWidth = 0.3f;
			Wheels[i].Mass = 20.0f;
			Wheels[i].DampingRate = 0.25f;

			Wheels[i].SusppensionLength = 0.2;
			Wheels[i].SusppensionStrength = 3500;
			Wheels[i].SusppensionDamping = 600;
		}
	}

	RaceVehicleMovementComponent::~RaceVehicleMovementComponent()
	{
		
	}

}