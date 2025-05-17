#pragma once

#if WITH_EDITOR

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

	protected:

		std::unique_ptr<LevelViewportGuiLayer> LevelViewportLayer;

	private:

		static std::unique_ptr<LevelViewport> SingletonInstance;

		friend class LevelViewportGuiLayer;
	};
}

#endif