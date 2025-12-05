#pragma once

#include "ForwardTypes.h"
#include <type_traits>
#include "Runtime/Renderer/RenderResource.h"

namespace Drn
{
	enum ViewSubresourceSubsetFlags
	{
		ViewSubresourceSubsetFlags_None = 0x0,
		ViewSubresourceSubsetFlags_DepthOnlyDsv = 0x1,
		ViewSubresourceSubsetFlags_StencilOnlyDsv = 0x2,
		ViewSubresourceSubsetFlags_DepthAndStencilDsv = (ViewSubresourceSubsetFlags_DepthOnlyDsv | ViewSubresourceSubsetFlags_StencilOnlyDsv),
	};

	struct CBufferView {};

	class CSubresourceSubset
	{
	public:
		CSubresourceSubset() {}
		inline explicit CSubresourceSubset(const CBufferView&) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_EndMip(1),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
		}
		inline explicit CSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_EndMip(1),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
			switch (Desc.ViewDimension)
			{
			default: break;

			case (D3D12_SRV_DIMENSION_BUFFER) :
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE1D) :
				m_BeginMip = uint8(Desc.Texture1D.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture1D.MipLevels);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE1DARRAY) :
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture1DArray.MipLevels);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2D) :
				m_BeginMip = uint8(Desc.Texture2D.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture2D.MipLevels);
				m_BeginPlane = uint8(Desc.Texture2D.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2D.PlaneSlice + 1);
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2DARRAY) :
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture2DArray.MipLevels);
				m_BeginPlane = uint8(Desc.Texture2DArray.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2DArray.PlaneSlice + 1);
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2DMS) :
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY) :
				m_BeginArray = uint16(Desc.Texture2DMSArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DMSArray.ArraySize);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURE3D) :
				m_EndArray = uint16(-1); //all slices
				m_BeginMip = uint8(Desc.Texture3D.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.Texture3D.MipLevels);
				break;

			case (D3D12_SRV_DIMENSION_TEXTURECUBE) :
				m_BeginMip = uint8(Desc.TextureCube.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.TextureCube.MipLevels);
				m_BeginArray = 0;
				m_EndArray = 6;
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;

			case (D3D12_SRV_DIMENSION_TEXTURECUBEARRAY) :
				m_BeginArray = uint16(Desc.TextureCubeArray.First2DArrayFace);
				m_EndArray = uint16(m_BeginArray + Desc.TextureCubeArray.NumCubes * 6);
				m_BeginMip = uint8(Desc.TextureCubeArray.MostDetailedMip);
				m_EndMip = uint8(m_BeginMip + Desc.TextureCubeArray.MipLevels);
				m_BeginPlane = GetPlaneSliceFromViewFormat(ResourceFormat, Desc.Format);
				m_EndPlane = m_BeginPlane + 1;
				break;
			}
		}
		inline explicit CSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
			switch (Desc.ViewDimension)
			{
			default: break;

			case (D3D12_UAV_DIMENSION_BUFFER) : break;

			case (D3D12_UAV_DIMENSION_TEXTURE1D) :
				m_BeginMip = uint8(Desc.Texture1D.MipSlice);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE1DARRAY) :
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MipSlice);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE2D) :
				m_BeginMip = uint8(Desc.Texture2D.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2D.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2D.PlaneSlice + 1);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE2DARRAY) :
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2DArray.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2DArray.PlaneSlice + 1);
				break;

			case (D3D12_UAV_DIMENSION_TEXTURE3D) :
				m_BeginArray = uint16(Desc.Texture3D.FirstWSlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture3D.WSize);
				m_BeginMip = uint8(Desc.Texture3D.MipSlice);
				break;
			}

			m_EndMip = m_BeginMip + 1;
		}
		inline explicit CSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC& Desc) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_BeginPlane(0),
			m_EndPlane(1)
		{
			switch (Desc.ViewDimension)
			{
			default: break;

			case (D3D12_RTV_DIMENSION_BUFFER) : break;

			case (D3D12_RTV_DIMENSION_TEXTURE1D) :
				m_BeginMip = uint8(Desc.Texture1D.MipSlice);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE1DARRAY) :
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MipSlice);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE2D) :
				m_BeginMip = uint8(Desc.Texture2D.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2D.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2D.PlaneSlice + 1);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE2DMS) : break;

			case (D3D12_RTV_DIMENSION_TEXTURE2DARRAY) :
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MipSlice);
				m_BeginPlane = uint8(Desc.Texture2DArray.PlaneSlice);
				m_EndPlane = uint8(Desc.Texture2DArray.PlaneSlice + 1);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY) :
				m_BeginArray = uint16(Desc.Texture2DMSArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DMSArray.ArraySize);
				break;

			case (D3D12_RTV_DIMENSION_TEXTURE3D) :
				m_BeginArray = uint16(Desc.Texture3D.FirstWSlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture3D.WSize);
				m_BeginMip = uint8(Desc.Texture3D.MipSlice);
				break;
			}

			m_EndMip = m_BeginMip + 1;
		}

		inline explicit CSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags) :
			m_BeginArray(0),
			m_EndArray(1),
			m_BeginMip(0),
			m_BeginPlane(0),
			m_EndPlane( D3D12GetFormatPlaneCount(Renderer::Get()->GetD3D12Device(), ResourceFormat))
		{
			switch (Desc.ViewDimension)
			{
			default: break;

			case (D3D12_DSV_DIMENSION_TEXTURE1D) :
				m_BeginMip = uint8(Desc.Texture1D.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE1DARRAY) :
				m_BeginArray = uint16(Desc.Texture1DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture1DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture1DArray.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE2D) :
				m_BeginMip = uint8(Desc.Texture2D.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE2DMS) : break;

			case (D3D12_DSV_DIMENSION_TEXTURE2DARRAY) :
				m_BeginArray = uint16(Desc.Texture2DArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DArray.ArraySize);
				m_BeginMip = uint8(Desc.Texture2DArray.MipSlice);
				break;

			case (D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY) :
				m_BeginArray = uint16(Desc.Texture2DMSArray.FirstArraySlice);
				m_EndArray = uint16(m_BeginArray + Desc.Texture2DMSArray.ArraySize);
				break;
			}

			m_EndMip = m_BeginMip + 1;

			if (m_EndPlane == 2)
			{
				if ((Flags & ViewSubresourceSubsetFlags_DepthAndStencilDsv) != ViewSubresourceSubsetFlags_DepthAndStencilDsv)
				{
					if (Flags & ViewSubresourceSubsetFlags_DepthOnlyDsv)
					{
						m_BeginPlane = 0;
						m_EndPlane = 1;
					}
					else if (Flags & ViewSubresourceSubsetFlags_StencilOnlyDsv)
					{
						m_BeginPlane = 1;
						m_EndPlane = 2;
					}
				}
			}
		}

		__forceinline bool DoesNotOverlap(const CSubresourceSubset& other) const
		{
			if (m_EndArray <= other.m_BeginArray)
			{
				return true;
			}

			if (other.m_EndArray <= m_BeginArray)
			{
				return true;
			}

			if (m_EndMip <= other.m_BeginMip)
			{
				return true;
			}

			if (other.m_EndMip <= m_BeginMip)
			{
				return true;
			}

			if (m_EndPlane <= other.m_BeginPlane)
			{
				return true;
			}

			if (other.m_EndPlane <= m_BeginPlane)
			{
				return true;
			}

			return false;
		}

	protected:
		uint16 m_BeginArray; // Also used to store Tex3D slices.
		uint16 m_EndArray; // End - Begin == Array Slices
		uint8 m_BeginMip;
		uint8 m_EndMip; // End - Begin == Mip Levels
		uint8 m_BeginPlane;
		uint8 m_EndPlane;
	};

	template <typename TDesc>
	class ResourceView;

	class CViewSubresourceSubset : public CSubresourceSubset
	{
		friend class ResourceView < D3D12_SHADER_RESOURCE_VIEW_DESC >;
		friend class ResourceView < D3D12_RENDER_TARGET_VIEW_DESC >;
		friend class ResourceView < D3D12_DEPTH_STENCIL_VIEW_DESC >;
		friend class ResourceView < D3D12_UNORDERED_ACCESS_VIEW_DESC >;

	public:
		CViewSubresourceSubset() {}
		inline explicit CViewSubresourceSubset(const CBufferView&)
			: CSubresourceSubset(CBufferView())
			, m_MipLevels(1)
			, m_ArraySlices(1)
			, m_MostDetailedMip(0)
			, m_ViewArraySize(1)
		{
		}

		inline CViewSubresourceSubset(uint32 Subresource, uint8 MipLevels, uint16 ArraySize, uint8 PlaneCount)
			: m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(PlaneCount)
		{
			if (Subresource < uint32(MipLevels) * uint32(ArraySize))
			{
				m_BeginArray = Subresource / MipLevels;
				m_EndArray = m_BeginArray + 1;
				m_BeginMip = Subresource % MipLevels;
				m_EndMip = m_EndArray + 1;
			}
			else
			{
				m_BeginArray = 0;
				m_BeginMip = 0;
				if (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
				{
					m_EndArray = ArraySize;
					m_EndMip = MipLevels;
				}
				else
				{
					m_EndArray = 0;
					m_EndMip = 0;
				}
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
		}

		inline CViewSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags /*Flags*/)
			: CSubresourceSubset(Desc, ResourceFormat)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(D3D12GetFormatPlaneCount(Renderer::Get()->GetD3D12Device(), ResourceFormat))
		{
			if (Desc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
			{
				drn_check(m_BeginArray == 0);
				m_EndArray = 1;
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		inline CViewSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags /*Flags*/)
			: CSubresourceSubset(Desc)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(D3D12GetFormatPlaneCount(Renderer::Get()->GetD3D12Device(), ResourceFormat))
		{
			if (Desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D)
			{
				m_BeginArray = 0;
				m_EndArray = 1;
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		inline CViewSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags)
			: CSubresourceSubset(Desc, ResourceFormat, Flags)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(D3D12GetFormatPlaneCount(Renderer::Get()->GetD3D12Device(), ResourceFormat))
		{
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		inline CViewSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags /*Flags*/)
			: CSubresourceSubset(Desc)
			, m_MipLevels(MipLevels)
			, m_ArraySlices(ArraySize)
			, m_PlaneCount(D3D12GetFormatPlaneCount(Renderer::Get()->GetD3D12Device(), ResourceFormat))
		{
			if (Desc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE3D)
			{
				m_BeginArray = 0;
				m_EndArray = 1;
			}
			m_MostDetailedMip = m_BeginMip;
			m_ViewArraySize = m_EndArray - m_BeginArray;
			Reduce();
		}

		template<typename T>
		static CViewSubresourceSubset FromView(const T* pView)
		{
			return CViewSubresourceSubset(
				pView->Desc(),
				static_cast<uint8>(pView->GetResource()->GetMipLevels()),
				static_cast<uint16>(pView->GetResource()->GetArraySize()),
				static_cast<uint8>(pView->GetResource()->GetPlaneCount())
				);
		}

	public:
		class CViewSubresourceIterator;

	public:
		CViewSubresourceIterator begin() const;
		CViewSubresourceIterator end() const;
		bool IsWholeResource() const;
		uint32 ArraySize() const;

		uint8 MostDetailedMip() const;
		uint16 ViewArraySize() const;

		uint32 MinSubresource() const;
		uint32 MaxSubresource() const;

	private:
		// Strictly for performance, allows coalescing contiguous subresource ranges into a single range
		inline void Reduce()
		{
			if (m_BeginMip == 0
				&& m_EndMip == m_MipLevels
				&& m_BeginArray == 0
				&& m_EndArray == m_ArraySlices
				&& m_BeginPlane == 0
				&& m_EndPlane == m_PlaneCount)
			{
				uint32 startSubresource = D3D12CalcSubresource(0, 0, m_BeginPlane, m_MipLevels, m_ArraySlices);
				uint32 endSubresource = D3D12CalcSubresource(0, 0, m_EndPlane, m_MipLevels, m_ArraySlices);

				// Only coalesce if the full-resolution UINTs fit in the UINT8s used for storage here
				if (endSubresource < static_cast<uint8>(-1))
				{
					m_BeginArray = 0;
					m_EndArray = 1;
					m_BeginPlane = 0;
					m_EndPlane = 1;
					m_BeginMip = static_cast<uint8>(startSubresource);
					m_EndMip = static_cast<uint8>(endSubresource);
				}
			}
		}

	protected:
		uint8 m_MipLevels;
		uint16 m_ArraySlices;
		uint8 m_PlaneCount;
		uint8 m_MostDetailedMip;
		uint16 m_ViewArraySize;
	};

	class CViewSubresourceSubset::CViewSubresourceIterator
	{
	public:
		inline CViewSubresourceIterator(CViewSubresourceSubset const& SubresourceSet, uint16 ArraySlice, uint8 PlaneSlice)
			: m_Subresources(SubresourceSet)
			, m_CurrentArraySlice(ArraySlice)
			, m_CurrentPlaneSlice(PlaneSlice)
		{
		}

		inline CViewSubresourceSubset::CViewSubresourceIterator& operator++()
		{
			drn_check(m_CurrentArraySlice < m_Subresources.m_EndArray);

			if (++m_CurrentArraySlice >= m_Subresources.m_EndArray)
			{
				drn_check(m_CurrentPlaneSlice < m_Subresources.m_EndPlane);
				m_CurrentArraySlice = m_Subresources.m_BeginArray;
				++m_CurrentPlaneSlice;
			}

			return *this;
		}

		inline CViewSubresourceSubset::CViewSubresourceIterator& operator--()
		{
			if (m_CurrentArraySlice <= m_Subresources.m_BeginArray)
			{
				m_CurrentArraySlice = m_Subresources.m_EndArray;

				drn_check(m_CurrentPlaneSlice > m_Subresources.m_BeginPlane);
				--m_CurrentPlaneSlice;
			}

			--m_CurrentArraySlice;

			return *this;
		}

		inline bool operator==(CViewSubresourceIterator const& other) const
		{
			return &other.m_Subresources == &m_Subresources
				&& other.m_CurrentArraySlice == m_CurrentArraySlice
				&& other.m_CurrentPlaneSlice == m_CurrentPlaneSlice;
		}

		inline bool operator!=(CViewSubresourceIterator const& other) const
		{
			return !(other == *this);
		}

		inline uint32 StartSubresource() const
		{
			return D3D12CalcSubresource(m_Subresources.m_BeginMip, m_CurrentArraySlice, m_CurrentPlaneSlice, m_Subresources.m_MipLevels, m_Subresources.m_ArraySlices);
		}

		inline uint32 EndSubresource() const
		{
			return D3D12CalcSubresource(m_Subresources.m_EndMip, m_CurrentArraySlice, m_CurrentPlaneSlice, m_Subresources.m_MipLevels, m_Subresources.m_ArraySlices);
		}

		inline std::pair<uint32, uint32> operator*() const
		{
			std::pair<uint32, uint32> NewPair;
			NewPair.first = StartSubresource();
			NewPair.second = EndSubresource();
			return NewPair;
		}

	private:
		CViewSubresourceSubset const& m_Subresources;
		uint16 m_CurrentArraySlice;
		uint8 m_CurrentPlaneSlice;
	};

	inline CViewSubresourceSubset::CViewSubresourceIterator CViewSubresourceSubset::begin() const
	{
		return CViewSubresourceIterator(*this, m_BeginArray, m_BeginPlane);
	}

	inline CViewSubresourceSubset::CViewSubresourceIterator CViewSubresourceSubset::end() const
	{
		return CViewSubresourceIterator(*this, m_BeginArray, m_EndPlane);
	}

	inline bool CViewSubresourceSubset::IsWholeResource() const
	{
		return m_BeginMip == 0 && m_BeginArray == 0 && m_BeginPlane == 0 && (m_EndMip * m_EndArray * m_EndPlane == m_MipLevels * m_ArraySlices * m_PlaneCount);
	}

	inline uint32 CViewSubresourceSubset::ArraySize() const
	{
		return m_ArraySlices;
	}

	inline uint8 CViewSubresourceSubset::MostDetailedMip() const
	{
		return m_MostDetailedMip;
	}

	inline uint16 CViewSubresourceSubset::ViewArraySize() const
	{
		return m_ViewArraySize;
	}

	inline uint32 CViewSubresourceSubset::MinSubresource() const
	{
		return (*begin()).first;
	}

	inline uint32 CViewSubresourceSubset::MaxSubresource() const
	{
		return (*(--end())).second;
	}

// -----------------------------------------------------------------------------------------------------------------------------------------

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

		void CreateViewWithCounter(const TDesc& Desc, ID3D12Resource* Resource, ID3D12Resource* CounterResource)
		{
			(Renderer::Get()->GetD3D12Device()->*TCreateViewMap<TDesc>::GetCreate()) (Resource, CounterResource, &Desc, m_CpuHandle);
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

	typedef TViewDescriptorHandle<D3D12_SHADER_RESOURCE_VIEW_DESC>	DescriptorHandleSRV;
	typedef TViewDescriptorHandle<D3D12_RENDER_TARGET_VIEW_DESC>	DescriptorHandleRTV;
	typedef TViewDescriptorHandle<D3D12_DEPTH_STENCIL_VIEW_DESC>	DescriptorHandleDSV;
	typedef TViewDescriptorHandle<D3D12_UNORDERED_ACCESS_VIEW_DESC>	DescriptorHandleUAV;

// ---------------------------------------------------------------------------------------------------------

	class ConstantBufferView
	{
	public:
		void Create(D3D12_GPU_VIRTUAL_ADDRESS GpuAddress, const uint32 AlignedSize)
		{
			m_Desc.BufferLocation = GpuAddress;
			m_Desc.SizeInBytes = AlignedSize;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView(&m_Desc, m_CpuHandle);
		}

		inline const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() const { return m_CpuHandle; }
		inline const CD3DX12_GPU_DESCRIPTOR_HANDLE& GetGpuHandle() const { return m_GpuHandle; }
		inline uint32 GetIndex() const { return m_Index; }

		inline void AllocateDescriptorSlot()
		{
			Renderer::Get()->m_BindlessSrvHeapAllocator.Alloc(&m_CpuHandle, &m_GpuHandle);
			m_Index = Renderer::Get()->GetBindlessSrvIndex(m_GpuHandle);
		}

		inline void FreeDescriptorSlot()
		{
			Renderer::Get()->m_BindlessSrvHeapAllocator.Free(m_CpuHandle, m_GpuHandle);
		}

		inline const D3D12_CONSTANT_BUFFER_VIEW_DESC& GetDesc() const { return m_Desc; }

	private:
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_Desc;
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
		CD3DX12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
		uint32 m_Index;
	};

// ---------------------------------------------------------------------------------------------------------

	template<typename TDesc>
	class ResourceView
	{
	private:
		TViewDescriptorHandle<TDesc> Descriptor;

	protected:
		ViewSubresourceSubsetFlags Flags;
		ResourceLocation* m_ResourceLocation;
		RenderResource* Resource;
		CViewSubresourceSubset ViewSubresourceSubset;

		TDesc Desc;

		ResourceView(Device* InParent, ViewSubresourceSubsetFlags InFlags)
			: Flags(InFlags)
//			, Descriptor(InParent)
		{
			Descriptor.AllocateDescriptorSlot(); // TODO: move to descriptor constructor
		}

		virtual ~ResourceView()
		{
			Descriptor.FreeDescriptorSlot();
		}

	private:
		void Initialize(const TDesc& InDesc, ResourceLocation& InResourceLocation)
		{
			m_ResourceLocation = &InResourceLocation;
			Resource = m_ResourceLocation->GetResource();
			drn_check(Resource);

			Desc = InDesc;

			ViewSubresourceSubset = CViewSubresourceSubset(Desc,
				Resource->GetMipLevels(),
				Resource->GetArraySize(),
				Resource->GetDesc().Format,
				Flags);
		}

	protected:
		void CreateView(const TDesc& InDesc, ResourceLocation& InResourceLocation)
		{
			Initialize(InDesc, InResourceLocation);

			ID3D12Resource* D3DResource = m_ResourceLocation->GetResource()->GetResource();
			Descriptor.CreateView(Desc, D3DResource);
		}

		void CreateViewWithCounter(const TDesc& InDesc, ResourceLocation& InResourceLocation, RenderResource* InCounterResource)
		{
			Initialize(InDesc, InResourceLocation);
		
			ID3D12Resource* D3DResource = m_ResourceLocation->GetResource()->GetResource();
			ID3D12Resource* D3DCounterResource = InCounterResource ? InCounterResource->GetResource() : nullptr;
			Descriptor.CreateViewWithCounter(Desc, D3DResource, D3DCounterResource);
		}

	public:
		//inline FD3D12Device*					GetParentDevice()			const { return Descriptor.GetParentDevice(); }
		//inline FD3D12Device*					GetParentDevice_Unsafe()	const { return Descriptor.GetParentDevice_Unsafe(); }
		inline const TDesc&						GetDesc()					const { return Desc; }
		inline CD3DX12_CPU_DESCRIPTOR_HANDLE	GetView()					const { return Descriptor.GetCpuHandle(); }
		inline uint32							GetDescriptorHeapIndex()	const { return Descriptor.GetIndex(); }
		inline RenderResource*					GetResource()				const { return Resource; }
		inline ResourceLocation*				GetResourceLocation()		const { return m_ResourceLocation; }
		inline const CViewSubresourceSubset&	GetViewSubresourceSubset()	const { return ViewSubresourceSubset; }

		//void SetParentDevice(FD3D12Device* InParent)
		//{
		//	Descriptor.SetParentDevice(InParent);
		//}

		template< class T >
		inline bool DoesNotOverlap(const ResourceView< T >& Other) const
		{
			return ViewSubresourceSubset.DoesNotOverlap(Other.GetViewSubresourceSubset());
		}
	};

// ---------------------------------------------------------------------------------------------------------

	class BaseShaderResourceView
	{
	protected:
		void Remove();

		friend class BaseShaderResource;
		BaseShaderResource* DynamicResource = nullptr;
	};

	class ShaderResourceView : public BaseShaderResourceView, public SimpleRenderResource, public ResourceView<D3D12_SHADER_RESOURCE_VIEW_DESC>
	{
		bool bContainsDepthPlane;
		bool bContainsStencilPlane;
		bool bSkipFastClearFinalize;
		bool bRequiresResourceStateTracking;
		uint32 Stride;

	public:
		ShaderResourceView(Device* InParent)
			: ResourceView(InParent, ViewSubresourceSubsetFlags_None)
		{}

		// Used for all other SRV resource types. Initialization is immediate on the calling thread.
		// Should not be used for dynamic resources which can be renamed.
		ShaderResourceView(Device* InParent, D3D12_SHADER_RESOURCE_VIEW_DESC& InDesc, ResourceLocation& InResourceLocation, uint32 InStride = -1, bool InSkipFastClearFinalize = false)
			: ShaderResourceView(InParent)
		{
			Initialize(InDesc, InResourceLocation, InStride, InSkipFastClearFinalize);
		}

		~ShaderResourceView()
		{
			BaseShaderResourceView::Remove();
		}

		void Initialize(D3D12_SHADER_RESOURCE_VIEW_DESC& InDesc, ResourceLocation& InResourceLocation, uint32 InStride, bool InSkipFastClearFinalize = false)
		{
			Stride = InStride;
			bContainsDepthPlane   = InResourceLocation.GetResource()->IsDepthStencilResource() && GetPlaneSliceFromViewFormat(InResourceLocation.GetResource()->GetDesc().Format, InDesc.Format) == 0;
			bContainsStencilPlane = InResourceLocation.GetResource()->IsDepthStencilResource() && GetPlaneSliceFromViewFormat(InResourceLocation.GetResource()->GetDesc().Format, InDesc.Format) == 1;
			bRequiresResourceStateTracking = InResourceLocation.GetResource()->RequiresResourceStateTracking();
			bSkipFastClearFinalize = InSkipFastClearFinalize;

			CreateView(InDesc, InResourceLocation);
		}

		void Initialize(Device* InParent, D3D12_SHADER_RESOURCE_VIEW_DESC& InDesc, ResourceLocation& InResourceLocation, uint32 InStride, bool InSkipFastClearFinalize = false)
		{
			//if (!this->GetParentDevice_Unsafe())
			//{
			//	// This is a null SRV created without viewing on any resource
			//	// We need to set its device and allocate a descriptor slot before moving forward
			//	this->SetParentDevice(InParent);
			//}
			//check(GetParentDevice() == InParent);
			Initialize(InDesc, InResourceLocation, InStride, InSkipFastClearFinalize);
		}

		void Rename(ResourceLocation& InResourceLocation)
		{
			//// Update the first element index, then reinitialize the SRV
			//if (Desc.ViewDimension == D3D12_SRV_DIMENSION_BUFFER)
			//{
			//	Desc.Buffer.FirstElement = InResourceLocation.GetOffsetFromBaseOfResource() / Stride;
			//}

			Initialize(Desc, InResourceLocation, Stride);
		}

		void Rename(float ResourceMinLODClamp)
		{
			drn_check(m_ResourceLocation);
			drn_check(Desc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D);

			Desc.Texture2D.ResourceMinLODClamp = ResourceMinLODClamp;
			CreateView(Desc, *m_ResourceLocation);
		}

		inline bool IsDepthStencilResource()		const { return bContainsDepthPlane || bContainsStencilPlane; }
		inline bool IsDepthPlaneResource()			const { return bContainsDepthPlane; }
		inline bool IsStencilPlaneResource()		const { return bContainsStencilPlane; }
		inline bool GetSkipFastClearFinalize()		const { return bSkipFastClearFinalize; }
		inline bool RequiresResourceStateTracking() const { return bRequiresResourceStateTracking; }
	};

	class UnorderedAccessView : public SimpleRenderResource, public ResourceView<D3D12_UNORDERED_ACCESS_VIEW_DESC>
	{
		UnorderedAccessView(Device* InParent, D3D12_UNORDERED_ACCESS_VIEW_DESC& InDesc, ResourceLocation& InResourceLocation, RenderResource* InCounterResource = nullptr)
			: ResourceView(InParent, ViewSubresourceSubsetFlags_None)
			, CounterResource(InCounterResource)
			, CounterResourceInitialized(false)
		{
			CreateViewWithCounter(InDesc, InResourceLocation, InCounterResource);
		}

		bool IsCounterResourceInitialized() const { return CounterResourceInitialized; }
		void MarkCounterResourceInitialized() { CounterResourceInitialized = true; }

		RenderResource* GetCounterResource() { return CounterResource; }

	private:

		TRefCountPtr<RenderResource> CounterResource;
		bool CounterResourceInitialized;
	};

	class RenderTargetView : public SimpleRenderResource, public ResourceView<D3D12_RENDER_TARGET_VIEW_DESC>
	{
	public:
		RenderTargetView(Device* InParent, const D3D12_RENDER_TARGET_VIEW_DESC& InRTVDesc, ResourceLocation& InResourceLocation)
			: ResourceView(InParent, ViewSubresourceSubsetFlags_None)
		{
			CreateView(InRTVDesc, InResourceLocation);
		}
	};

	class DepthStencilView : public SimpleRenderResource, public ResourceView<D3D12_DEPTH_STENCIL_VIEW_DESC>
	{
		const bool bHasDepth;
		const bool bHasStencil;
		CViewSubresourceSubset DepthOnlyViewSubresourceSubset;
		CViewSubresourceSubset StencilOnlyViewSubresourceSubset;

	public:
		DepthStencilView(Device* InParent, const D3D12_DEPTH_STENCIL_VIEW_DESC& InDSVDesc, ResourceLocation& InResourceLocation, bool InHasStencil)
			: ResourceView(InParent, ViewSubresourceSubsetFlags_DepthAndStencilDsv)
			, bHasDepth(true)
			, bHasStencil(InHasStencil)
		{
			CreateView(InDSVDesc, InResourceLocation);
			drn_check(Resource);

			if (bHasDepth)
			{
				DepthOnlyViewSubresourceSubset = CViewSubresourceSubset(InDSVDesc,
					Resource->GetMipLevels(),
					Resource->GetArraySize(),
					Resource->GetDesc().Format,
					ViewSubresourceSubsetFlags_DepthOnlyDsv);
			}

			if (bHasStencil)
			{
				StencilOnlyViewSubresourceSubset = CViewSubresourceSubset(InDSVDesc,
					Resource->GetMipLevels(),
					Resource->GetArraySize(),
					Resource->GetDesc().Format,
					ViewSubresourceSubsetFlags_StencilOnlyDsv);
			}
		}

		bool HasDepth() const
		{
			return bHasDepth;
		}

		bool HasStencil() const
		{
			return bHasStencil;
		}

		CViewSubresourceSubset& GetDepthOnlyViewSubresourceSubset()
		{
			drn_check(bHasDepth);
			return DepthOnlyViewSubresourceSubset;
		}

		CViewSubresourceSubset& GetStencilOnlyViewSubresourceSubset()
		{
			drn_check(bHasStencil);
			return StencilOnlyViewSubresourceSubset;
		}
	};
}