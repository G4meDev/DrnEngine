#pragma once

namespace Drn
{
	class D3D12Scene;

	class Renderer
	{
	public:
		
		Renderer(){};

		static void Init(HINSTANCE inhInstance);
		static void Shutdown();

		static Renderer* Get();

		void Tick(float DeltaTime);

	protected:
		static Renderer* SingletonInstance;

		D3D12Scene* MainScene;

	private:
		
		void CreateMainScene();
	};
}