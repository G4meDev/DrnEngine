#pragma once

namespace Drn
{
	class D3D12Device;

	class D3D12RHI
	{
	public:

		static void Init();
		static void Shutdown();
		static D3D12RHI* Get();

		void CreateCommandQueue(D3D12Device* Device, const D3D12_COMMAND_QUEUE_DESC& Desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>& OutCommandQueue);

	protected:

		static D3D12RHI* SingletonInstance;

	private:
	};

}