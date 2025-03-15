#pragma once

namespace Drn
{
	class D3D12RHI;
	class D3D12Adapter;
	class D3D12Device;

	class Renderer
	{
	public:
		
		Renderer(){};

		static void Init();
		static void Shutdown();

		static Renderer* Get();
		static D3D12RHI* GetRHI();

		void Tick(float DeltaTime);

		D3D12Adapter* Adapter;
		D3D12Device* MainDevice;

	protected:
		static Renderer* SingletonInstance;

	private:

	};
}