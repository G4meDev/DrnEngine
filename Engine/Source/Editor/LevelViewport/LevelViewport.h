#pragma once

#if WITH_EDITOR

LOG_DECLARE_CATEGORY(LogLevelViewport);

namespace Drn
{
	class LevelViewportGuiLayer;

	class LevelViewport
	{
	public:

		LevelViewport( World* InOwningWorld );
		~LevelViewport();

		static void Init( World* InOwningWorld );
		static void Shutdown();
		static void Tick( float DeltaTime );

		static LevelViewport* Get();

		void OnSelectedNewComponent( Component* NewComponent );
		Component* GetSelectedComponent() { return m_SelectedComponent; }

		void OnRemovedActorsFromWorld( std::vector<Actor*> RemovedActors );

	protected:

		std::unique_ptr<LevelViewportGuiLayer> LevelViewportLayer;

		Component* m_SelectedComponent;
		World* m_OwningWorld;

	private:

		static LevelViewport* SingletonInstance;

		friend class LevelViewportGuiLayer;
	};
}

#endif