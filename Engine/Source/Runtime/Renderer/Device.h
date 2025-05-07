#pragma once

#include "ForwardTypes.h"
#include <dxgi1_6.h>

LOG_DECLARE_CATEGORY(LogDevice);

namespace Drn
{
	class Device
	{
	public:

		Device();
		~Device();

		inline ID3D12Device* GetD3D12Device() const { return m_Device.Get(); }

	private:

		Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
		DXGI_ADAPTER_DESC3 m_Description;
	};
}