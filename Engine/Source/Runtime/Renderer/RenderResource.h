#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/RefCounting.h"
#include "Runtime/Renderer/ResourceState.h"

#define DEFER_DELETE_PAGE_SIZE 64

namespace Drn
{
	struct BuddyAllocatorData
	{
		uint32 Offset;
		uint32 Order;

		void Init()
		{
			Offset = 0;
			Order = 0;
		}
	};

	class SimpleRenderResource : IRefCountedObject
	{
	public:
		SimpleRenderResource(bool InDeferDelete = true)
			: bDeferDelete(InDeferDelete)
			, NumRefs(0)
		{};

		virtual ~SimpleRenderResource() {}

		uint32 AddRef() const override
		{
			return ++NumRefs;
		}

		uint32 Release() const override
		{
			uint32 Refs = --NumRefs;
			if(Refs == 0)
			{
				if (bDeferDelete)
				{
					DeferDelete();
				}
				else
				{
					delete this;
				}
			}
			return Refs;
		}

		uint32 GetRefCount() const override
		{
			return NumRefs;
		}

		static void FlushPendingDeletes( bool bFlushDeferredDeletes = false );

	private:

		void DeferDelete() const
		{
			SimpleRenderResource* MutablePtr = const_cast<SimpleRenderResource*>(this);
			MutablePtr->bMarkedForDelete = true;

			for (ResourcesToDelete& ToDeletePage : DeferredDeletionQueue)
			{
				if (ToDeletePage.FrameDeleted == CurrentFrame && ToDeletePage.Resources.size() < DEFER_DELETE_PAGE_SIZE)
				{
					ToDeletePage.Resources.push_back(MutablePtr);
					return;
				}
			}

			ResourcesToDelete& ToDeletePage = DeferredDeletionQueue.emplace_back(CurrentFrame);
			ToDeletePage.Resources.push_back(MutablePtr);
		}

		mutable uint32 NumRefs;
		bool bDeferDelete : 1;
		bool bMarkedForDelete : 1;

		struct ResourcesToDelete
		{
			ResourcesToDelete(uint32 InFrameDeleted = 0)
				: FrameDeleted(InFrameDeleted)
			{}

			std::vector<SimpleRenderResource*>	Resources;
			uint32								FrameDeleted;
		};

		static std::vector<ResourcesToDelete> DeferredDeletionQueue;
		static uint32 CurrentFrame;
	};

	struct ResourceTypeHelper
	{
		ResourceTypeHelper(D3D12_RESOURCE_DESC& Desc, D3D12_HEAP_TYPE HeapType) :
			bSRV((Desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0),
			bDSV((Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0),
			bRTV((Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0),
			bUAV((Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0),
			bWritable(bDSV || bRTV || bUAV),
			bSRVOnly(bSRV && !bWritable),
			bBuffer(Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER),
			bReadBackResource(HeapType == D3D12_HEAP_TYPE_READBACK)
		{}

		const D3D12_RESOURCE_STATES GetOptimalInitialState(bool bAccurateWriteableStates) const
		{
			if (bSRVOnly)
			{
				return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}
			else if (bBuffer && !bUAV)
			{
				return (bReadBackResource) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (bWritable)
			{
				if (bAccurateWriteableStates)
				{
					if (bDSV)
					{
						return D3D12_RESOURCE_STATE_DEPTH_WRITE;
					}
					else if (bRTV)
					{
						return D3D12_RESOURCE_STATE_RENDER_TARGET;
					}
					else if (bUAV)
					{
						return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
					}
				}
				else
				{
					// This things require tracking anyway
					return D3D12_RESOURCE_STATE_COMMON;
				}
			}

			return D3D12_RESOURCE_STATE_COMMON;
		}

		const uint32 bSRV : 1;
		const uint32 bDSV : 1;
		const uint32 bRTV : 1;
		const uint32 bUAV : 1;
		const uint32 bWritable : 1;
		const uint32 bSRVOnly : 1;
		const uint32 bBuffer : 1;
		const uint32 bReadBackResource : 1;
	};

	class RenderResource : public RefCountedObject, public DeviceChild
	{
	private:
		TRefCountPtr<ID3D12Resource> Resource;

		D3D12_RESOURCE_DESC Desc;
		uint8 PlaneCount;
		uint16 SubresourceCount;
		ResourceState_New ResourceState;

		bool bRequiresResourceStateTracking : 1;
		bool bDepthStencil : 1;
		bool bDeferDelete : 1;
		bool bBackBuffer : 1;

		D3D12_HEAP_TYPE HeapType;
		D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress;
		void* ResourceBaseAddress;
		int32 NumMapCalls = 0;
		std::string DebugName;

#if D3D12_Debug_INFO
		static std::atomic<int64> TotalResourceCount;
		static std::atomic<int64> NoStateTrackingResourceCount;
#endif

	public:
		RenderResource(Device* ParentDevice,
			ID3D12Resource* InResource,
			D3D12_RESOURCE_DESC const& InDesc,
			D3D12_RESOURCE_STATES InInitialResourceState,
			bool InNeedsStateTracking,
			D3D12_HEAP_TYPE InHeapType = D3D12_HEAP_TYPE_DEFAULT);

		virtual ~RenderResource();

		operator ID3D12Resource&() { return *Resource; }
		ID3D12Resource* GetResource() const { return Resource.GetReference(); }

		void* Map(const D3D12_RANGE* ReadRange = nullptr);
		void Unmap();

		D3D12_RESOURCE_DESC const& GetDesc() const { return Desc; }
		D3D12_HEAP_TYPE GetHeapType() const { return HeapType; }
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return GPUVirtualAddress; }
		void* GetResourceBaseAddress() const { drn_check(ResourceBaseAddress); return ResourceBaseAddress; }
		uint16 GetMipLevels() const { return Desc.MipLevels; }
		uint16 GetArraySize() const { return (Desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? 1 : Desc.DepthOrArraySize; }
		uint8 GetPlaneCount() const { return PlaneCount; }
		uint16 GetSubresourceCount() const { return SubresourceCount; }
		ResourceState_New& GetResourceState() { drn_check(bRequiresResourceStateTracking); return ResourceState; }

		bool RequiresResourceStateTracking() const { return bRequiresResourceStateTracking; }

		inline bool IsBackBuffer() const { return bBackBuffer; }
		inline void SetIsBackBuffer(bool bBackBufferIn) { bBackBuffer = bBackBufferIn; }

		void SetName(const std::string& Name);
		std::string GetName() const { return DebugName;}

		void DoNotDeferDelete() { bDeferDelete = false; }
		inline bool ShouldDeferDelete() const { return bDeferDelete; }
		void DeferDelete();

		inline bool IsDepthStencilResource() const { return bDepthStencil; }

		void ReleaseResource();

#if RENDER_STATS
		static int64 GetTotalResourceCount() { return TotalResourceCount.load(); }
#endif
	};

	class ResourceLocation : public DeviceChild, public Noncopyable
	{
	public:

		enum class ResourceLocationType
		{
			eUndefined,
			eStandAlone,
			eSubAllocation,
			eFastAllocation,
			eMultiFrameFastAllocation,
			eAliased,
			eNodeReference,
			eHeapAliased, 
		};

		enum EAllocatorType : uint8
		{
			AT_Default,
			AT_SegList,
			AT_Unknown = 0xff
		};

		ResourceLocation(Device* Parent);
		~ResourceLocation();

		void Clear();
		static void TransferOwnership(ResourceLocation& Destination, ResourceLocation& Source);

		void SetResource(class RenderResource* Value);
		inline void SetType(ResourceLocationType Value) { Type = Value;}

		inline void SetOffsetFromBaseOfResource(uint64 Value) { OffsetFromBaseOfResource = Value; }
		inline void SetMappedBaseAddress(void* Value) { MappedBaseAddress = Value; }
		inline void SetGPUVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS Value) { GPUVirtualAddress = Value; }
		inline void SetAllocator(class RenderBuddyAllocator* Value) { Allocator = Value; }
		inline void SetSize(uint64 Value) { Size = Value; }

		inline ResourceLocationType GetType() const { return Type; }
		inline class RenderResource* GetResource() const { return UnderlyingResource; }
		inline uint64 GetOffsetFromBaseOfResource() const { return OffsetFromBaseOfResource; }
		inline void* GetMappedBaseAddress() const { return MappedBaseAddress; }
		inline BuddyAllocatorData& GetBuddyAllocatorData() { return AllocatorData; }
		inline RenderBuddyAllocator* GetAllocator() { ; return Allocator; }
		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return GPUVirtualAddress; }
		inline uint64 GetSize() const { return Size; }

		const inline bool IsValid() const { return Type != ResourceLocationType::eUndefined; }

		void AsStandAlone(RenderResource* Resource, uint32 BufferSize = 0, bool bInIsTransient = false );

		void SetTransient(bool bInTransient)
		{
			bTransient = bInTransient;
		}
		bool IsTransient() const
		{
			return bTransient;
		}

	private:

		template<bool bReleaseResource>
		void InternalClear();

		void ReleaseResource();

		ResourceLocationType Type;
		RenderResource* UnderlyingResource;
		class RenderBuddyAllocator* Allocator;
		BuddyAllocatorData AllocatorData;

		UINT64 OffsetFromBaseOfResource;
		void* MappedBaseAddress;
		D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress;

		uint64 Size;
		bool bTransient;
	};

	class BaseShaderResource : public DeviceChild, public IRefCountedObject
	{
	protected:
		std::vector<class BaseShaderResourceView*> DynamicSRVs;

	public:
		RenderResource* GetResource() const { return m_ResourceLocation.GetResource(); }

		void AddDynamicSRV(class BaseShaderResourceView* InSRV);
		void RemoveDynamicSRV(class BaseShaderResourceView* InSRV);
		void RemoveAllDynamicSRVs();

		ResourceLocation m_ResourceLocation;
		uint32 BufferAlignment;

	public:
		BaseShaderResource(Device* InParent)
			: DeviceChild(InParent)
			, m_ResourceLocation(InParent)
			, BufferAlignment(0)
		{}

		~BaseShaderResource();
	};


}