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
		inline ID3D12Device* GetD3DDevice() {return D3DDevice.Get(); }
		inline IDXGIFactory2* GetFactory() { return DxgiFactory.Get(); }

		inline std::vector<D3D12Scene*>& GetViewports() { return Viewports; }

	protected:
		bool bDebugDevice;

		Microsoft::WRL::ComPtr<IDXGIFactory2> DxgiFactory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> DxgiAdapter;

		Microsoft::WRL::ComPtr<ID3D12Device> D3DDevice;

		D3D12Device* Device;

		std::vector<D3D12Scene*> Viewports;

	private:
		void CreateDXGIFactory(bool bWithDebug);

		HRESULT EnumAdapters(IDXGIAdapter** TempAdapter) const;
	};
}