#pragma once

#include "ForwardTypes.h"
#include <type_traits>

namespace Drn
{
	template <typename TDesc>
	class TViewDescriptorHandle
	{
		template <typename TDesc> struct TCreateViewMap;
		template<> struct TCreateViewMap<D3D12_SHADER_RESOURCE_VIEW_DESC>	{ static decltype(&ID3D12Device::CreateShaderResourceView)	GetCreate()	{ return &ID3D12Device::CreateShaderResourceView;	} };
		template<> struct TCreateViewMap<D3D12_RENDER_TARGET_VIEW_DESC>		{ static decltype(&ID3D12Device::CreateRenderTargetView)	GetCreate()	{ return &ID3D12Device::CreateRenderTargetView;		} };
		template<> struct TCreateViewMap<D3D12_DEPTH_STENCIL_VIEW_DESC>		{ static decltype(&ID3D12Device::CreateDepthStencilView)	GetCreate()	{ return &ID3D12Device::CreateDepthStencilView;		} };
		template<> struct TCreateViewMap<D3D12_UNORDERED_ACCESS_VIEW_DESC>	{ static decltype(&ID3D12Device::CreateUnorderedAccessView)	GetCreate()	{ return &ID3D12Device::CreateUnorderedAccessView;	} };

	public:

		void CreateView(const TDesc& Desc, ID3D12Resource* Resource)
		{
			(Renderer::Get()->GetD3D12Device()->*TCreateViewMap<TDesc>::GetCreate()) (Resource, &Desc, m_CpuHandle);
		}

		void CreateUnorderedView(const TDesc& Desc, ID3D12Resource* Resource)
		{
			(Renderer::Get()->GetD3D12Device()->*TCreateViewMap<TDesc>::GetCreate()) (Resource, nullptr, &Desc, m_CpuHandle);
		}

		inline const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() const { return m_CpuHandle; }
		inline const CD3DX12_GPU_DESCRIPTOR_HANDLE& GetGpuHandle() const { return m_GpuHandle; }
		inline uint32 GetIndex() const { return m_Index; }

		inline void AllocateDescriptorSlot()
		{
			if constexpr (std::is_same_v<TDesc, D3D12_RENDER_TARGET_VIEW_DESC>)
			{
				Renderer::Get()->m_BindlessRTVHeapAllocator.Alloc(&m_CpuHandle, &m_GpuHandle);
				//m_Index = Renderer::Get()->GetBindlessRTVIndex(m_GpuHandle);
			}

			else if constexpr (std::is_same_v<TDesc, D3D12_DEPTH_STENCIL_VIEW_DESC>)
			{
				__debugbreak();
			}

			else
			{
				Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc(&m_CpuHandle, &m_GpuHandle);
				m_Index = Renderer::Get()->GetBindlessSrvIndex(m_GpuHandle);
			}
		}

		inline void FreeDescriptorSlot()
		{
			if constexpr (std::is_same_v<TDesc, D3D12_RENDER_TARGET_VIEW_DESC>)
			{
				Renderer::Get()->m_BindlessRTVHeapAllocator.Free(m_CpuHandle, m_GpuHandle);
			}

			else if constexpr (std::is_same_v<TDesc, D3D12_DEPTH_STENCIL_VIEW_DESC>)
			{
				__debugbreak();
			}

			else
			{
				Renderer::Get()->m_BindlessSrvHeapAllocator.Free(m_CpuHandle, m_GpuHandle);
			}
		}

	private:
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
		uint32 m_Index;
	};

	typedef TViewDescriptorHandle<D3D12_SHADER_RESOURCE_VIEW_DESC>                  DescriptorHandleSRV;
	typedef TViewDescriptorHandle<D3D12_RENDER_TARGET_VIEW_DESC>			        DescriptorHandleRTV;
	typedef TViewDescriptorHandle<D3D12_DEPTH_STENCIL_VIEW_DESC>                    DescriptorHandleDSV;
	typedef TViewDescriptorHandle<D3D12_UNORDERED_ACCESS_VIEW_DESC>                 DescriptorHandleUAV;

	//template<typename TDesc>
	//class ResourceView
	//{
	//public:
	//
	//private:
	//	TD3D12ViewDescriptorHandle<TDesc> Descriptor;
	//	Resource* Resource;
	//	TDesc Desc;
	//};
}