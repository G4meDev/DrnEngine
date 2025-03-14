#pragma once

class D3D12Adapter;
class D3D12Device;

namespace Drn
{
	class Renderer
	{
	public:
		
		Renderer(){};

		static void MakeInstance();
		static Renderer* Get();

		void Init();
		void Shutdown();
		void Tick(float DeltaTime);

		D3D12Adapter* Adapter;
		D3D12Device* MainDevice;

	protected:
		static Renderer* SingletonInstance;

	private:

	};
}


