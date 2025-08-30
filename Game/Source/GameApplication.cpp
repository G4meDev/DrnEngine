#include "GamePCH.h"
#include "GameApplication.h"
#include <iostream>

#include "Editor/Misc/EditorMisc.h"
#include "Player/TestPlayerCharacter.h"

namespace Drn
{
	void GameApplication::Startup()
	{
		REGISTER_LEVEL_SPAWNABLE_CLASS( TestPlayerCharacter, Game );
		REGISTER_SERIALIZABLE_ACTOR( EGameActorType::TestPlayerCharacter , TestPlayerCharacter );

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