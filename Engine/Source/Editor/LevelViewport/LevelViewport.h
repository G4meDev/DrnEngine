#pragma once

#if WITH_EDITOR

LOG_DECLARE_CATEGORY(LogLevelViewport);

namespace Drn
{
	class LevelViewportGuiLayer;

	class LevelViewport
	{
	public:

		LevelViewport();
		~LevelViewport();

		void Init();
		void Shutdown();
		void Tick( float DeltaTime );

		static LevelViewport* Get();

		void OnSelectedNewComponent( Component* NewComponent );
		Component* GetSelectedComponent() { return m_SelectedComponent; }

	protected:

		std::unique_ptr<LevelViewportGuiLayer> LevelViewportLayer;

		Component* m_SelectedComponent;

	private:

		static std::unique_ptr<LevelViewport> SingletonInstance;

		friend class LevelViewportGuiLayer;
	};
}

#endif