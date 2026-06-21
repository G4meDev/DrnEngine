#pragma once

#include "Runtime/Core/Application.h"

namespace Drn
{
	enum EGameCollisionChannel
	{
		ECC_GameTestChannel = NUM_ENGINE_COLLISION_CHANNELS,

		ECC_GAME_MAX,
	};

	enum class EGamePhysicalSurface : uint8
	{
		SurfaceType_Default = 0,
		SurfaceType_Grass
	};

	class GameApplication : public Application
	{
	public:
	

	protected:
		virtual void Startup() override;
		virtual void Shutdown() override;
	
		virtual void Tick(float DeltaTime) override;

	};
}
