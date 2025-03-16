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
		ID3D12Device* GetD3DDevice();



	protected:

		D3D12Adapter* Adapter;

		void Initialize();

		friend D3D12Adapter;

	private:
	};

}