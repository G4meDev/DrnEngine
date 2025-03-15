#pragma once

namespace Drn
{
	class D3D12Adapter;
	class D3D12Queue;

	class D3D12Device
	{
	public:
		D3D12Device(D3D12Adapter* InAdapter);

		inline D3D12Adapter* GetAdapter(){ return Adapter; }
		inline ID3D12Device* GetDevice(){ return Device.Get(); }

	protected:

		D3D12Adapter* Adapter;
		Microsoft::WRL::ComPtr<ID3D12Device> Device;

		D3D12Queue* CommandQueue_Direct;
		D3D12Queue* CommandQueue_Copy;
		D3D12Queue* CommandQueue_Compute;

	private:
	};

}