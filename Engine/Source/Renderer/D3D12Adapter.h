#pragma once

namespace Drn
{
	class D3D12Device;

	class D3D12Adapter
	{
	public:
		D3D12Adapter();

	public:
		void InitalizeAdapter(bool bWithDebug);

		inline IDXGIAdapter* GetAdapter() {return DxgiAdapter.Get(); }

	protected:
		bool bDebugDevice;

		Microsoft::WRL::ComPtr<IDXGIFactory2> DxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> DxgiAdapter;

	private:
		//void CreateDevice(D3D12Device* Device);
		void CreateDXGIFactory(bool bWithDebug);

		HRESULT EnumAdapters(IDXGIAdapter** TempAdapter) const;
	};
}