#include "DrnPCH.h"
#include "D3D12Adapter.h"
#include "D3D12Device.h"

namespace Drn
	{
	D3D12Adapter::D3D12Adapter()
	{
		DxgiFactory = nullptr;
		DxgiAdapter = nullptr;

#if DRN_DEBUG
		bDebugDevice = true;
#else
		bDebugDevice = false;
#endif

		InitalizeAdapter(bDebugDevice);
		CreateMainDevice();
	}

	void D3D12Adapter::InitalizeAdapter(bool bWithDebug)
	{
		if (bWithDebug)
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
			HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(DebugController.GetAddressOf()));
			if (SUCCEEDED(hr))
			{
				DebugController->EnableDebugLayer();
				bDebugDevice = true;
			}
		}

		CreateDXGIFactory(bWithDebug);

		Microsoft::WRL::ComPtr<IDXGIAdapter> TempAdapter;
		EnumAdapters(TempAdapter.GetAddressOf());
		TempAdapter->QueryInterface(IID_PPV_ARGS(DxgiAdapter.GetAddressOf()));
	}

	void D3D12Adapter::CreateMainDevice()
	{
		Device = new D3D12Device(this);
		D3D12CreateDevice(GetAdapter(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(Device->Device.GetAddressOf()));
		Device->Initialize();
	}

	void D3D12Adapter::CreateDXGIFactory(bool bWithDebug)
	{
		const uint32 Flags = bWithDebug ? DXGI_CREATE_FACTORY_DEBUG : 0;
		CreateDXGIFactory2(Flags, IID_PPV_ARGS(DxgiFactory.GetAddressOf()));
	}

	HRESULT D3D12Adapter::EnumAdapters(IDXGIAdapter** TempAdapter) const
	{
		int AdaptorIndex = -1;

		SIZE_T MaxDedicatedVideoMemory = 0;
		for (int i = 0; DxgiFactory->EnumAdapters(i, TempAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC AdaptorDesc;
			(*TempAdapter)->GetDesc(&AdaptorDesc);

			if (AdaptorDesc.DedicatedVideoMemory > MaxDedicatedVideoMemory)
			{
				MaxDedicatedVideoMemory = AdaptorDesc.DedicatedVideoMemory;
				AdaptorIndex = i;
			}
		}

		return DxgiFactory->EnumAdapters(AdaptorIndex, TempAdapter);
	}
}