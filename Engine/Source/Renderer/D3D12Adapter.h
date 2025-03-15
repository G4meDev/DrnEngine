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
		void CreateMainDevice();

		inline IDXGIAdapter* GetAdapter() {return DxgiAdapter.Get(); }
		inline D3D12Device* GetDevice() {return Device; }

		inline std::vector<D3D12Viewport*>& GetViewports() { return Viewports; }

	protected:
		bool bDebugDevice;

		Microsoft::WRL::ComPtr<IDXGIFactory2> DxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> DxgiAdapter;

		D3D12Device* Device;

		std::vector<D3D12Viewport*> Viewports;

	private:
		void CreateDXGIFactory(bool bWithDebug);

		HRESULT EnumAdapters(IDXGIAdapter** TempAdapter) const;
	};
}