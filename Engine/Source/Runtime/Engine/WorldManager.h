#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnOpenLevelDelegate, World*)
	DECLARE_MULTICAST_DELEGATE_OneParam( OnCloseLevelDelegate, World*)

	class WorldManager
	{
	public:

		WorldManager();

		static void Init();
		static void Shutdown();

		inline static WorldManager* Get() { return m_SingletonInstance; }

		World* AllocateWorld();
		void ReleaseWorld(World*& InWorld);

		void Tick(float DeltaTime);

		void LoadInitalWorld();

		inline World* GetMainWorld() { return m_MainWorld; }

		void LoadLevel(const std::string& LevelPath );

		OnOpenLevelDelegate OnOpenLevel;
		OnCloseLevelDelegate OnCloseLevel;


#if WITH_EDITOR

		void StartPlayInEditor();
		void EndPlayInEditor();

#endif

	protected:

		static WorldManager* m_SingletonInstance;

		std::set<World*> m_AllocatedWorlds;

		World* m_PendingLevel;

		World* m_MainWorld;
		void LoadDefaultWorld();

		std::string m_LastLoadedLevel = "";

		friend class LevelViewportGuiLayer;
		friend class Editor;

	private:
	};
}