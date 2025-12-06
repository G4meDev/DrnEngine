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
		inline explicit CSubresourceSubset(const CBufferView&);
		inline explicit CSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat);
		inline explicit CSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc);
		inline explicit CSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC& Desc);
		inline explicit CSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags);

		bool DoesNotOverlap(const CSubresourceSubset& other) const;

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

		CViewSubresourceSubset(uint32 Subresource, uint8 MipLevels, uint16 ArraySize, uint8 PlaneCount);
		CViewSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags);
		CViewSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags);
		CViewSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags);
		CViewSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC& Desc, uint8 MipLevels, uint16 ArraySize, DXGI_FORMAT ResourceFormat, ViewSubresourceSubsetFlags Flags);

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
		void Reduce();

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

		uint32 StartSubresource() const;
		uint32 EndSubresource() const;
		std::pair<uint32, uint32> operator*() const;

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

			ViewSubresourceSubset = CViewSubresourceSubset(Desc, Resource->GetMipLevels(), Resource->GetArraySize(), Resource->GetDesc().Format, Flags);
		}

	protected:
		void CreateView(const TDesc& InDesc, ResourceLocation& InResourceLocation)
		{
			Initialize(InDesc, InResourceLocation);

			ID3D12Resource* D3DResource = m_ResourceLocation->GetResource()->GetResource();
			Descriptor.CreateView(Desc, D3DResource);
		}

		void CreateViewWithCounter(const TDesc& InDesc, ResourceLocation& InResourceLocation, class RenderResource* InCounterResource)
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