#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class WorldManager
	{
	public:
	
		WorldManager();

		static void Init();
		static void Shutdown();

		inline static WorldManager* Get() { return m_SingletonInstance; }

		World* AllocateWorld();
		void RemoveWorld(World* InWorld);
		void RemoveAndInvalidateWorld(World*& InWorld);

		void Tick(float DeltaTime);

	protected:

		static WorldManager* m_SingletonInstance;

		std::set<World*> m_AllocatedWorlds;

	private:
	};
}