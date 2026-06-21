#include "GamePCH.h"
#include "GameApplication.h"
#include <iostream>

#include "Editor/Misc/EditorMisc.h"
#include "Player/TestPlayerCharacter.h"
#include "Vehicle/RaceVehiclePawn.h"

namespace Drn
{
	void RegisterGameTypes()
	{
		REGISTER_LEVEL_SPAWNABLE_CLASS( TestPlayerCharacter, Game );
		REGISTER_SERIALIZABLE_ACTOR( EGameActorType::TestPlayerCharacter , TestPlayerCharacter );

		REGISTER_LEVEL_SPAWNABLE_CLASS( RaceVehiclePawn, Game );
		REGISTER_SERIALIZABLE_ACTOR( EGameActorType::RaceVehiclePawn , RaceVehiclePawn);

		EngineTypes::Get()->SurfaceTypesDisplayNames[(int32)EGamePhysicalSurface::SurfaceType_Grass] = "Grass";

		EngineTypes::Get()->CollisionChannelDisplayNames[ECC_GameTestChannel] = "GameTest";

		CollisionResponseTemplate GameTestProfile("GameTest", ECollisionEnabled::QueryAndPhysics, static_cast<ECollisionChannel>(ECC_GameTestChannel));
		GameTestProfile.ResponseToChannels.SetAllChannels(ECR_Block);
		CollisionProfile::Get()->AddProfile(GameTestProfile);
		CollisionProfile::Get()->UpdateProfileChannelResponse("BlockAll", static_cast<ECollisionChannel>(ECC_GameTestChannel), ECR_Ignore);
	}

	void GameApplication::Startup()
	{
		Application::Startup();

	}

	void GameApplication::Shutdown()
	{
		Application::Shutdown();
	}

	void GameApplication::Tick(float DeltaTime)
	{
		Application::Tick(DeltaTime);
	}

}