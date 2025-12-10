#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderCommon.h"

#define MIN_PLACED_BUFFER_SIZE ( 64 * 1024 )
#define D3D_BUFFER_ALIGNMENT   ( 64 * 1024 )

#if D3D12_RHI_RAYTRACING
    #define DEFAULT_BUFFER_POOL_MAX_ALLOC_SIZE    ( 64 * 1024 * 1024 )
    #define DEFAULT_BUFFER_POOL_DEFAULT_POOL_SIZE ( 16 * 1024 * 1024 )
#else
    #define DEFAULT_BUFFER_POOL_MAX_ALLOC_SIZE    ( 64 * 1024 )
    #define DEFAULT_BUFFER_POOL_DEFAULT_POOL_SIZE ( 8 * 1024 * 1024 )
#endif

#define READBACK_BUFFER_POOL_MAX_ALLOC_SIZE    ( 64 * 1024 )
#define READBACK_BUFFER_POOL_DEFAULT_POOL_SIZE ( 4 * 1024 * 1024 )

namespace Drn
{
	class RenderResourceAllocator : public DeviceChild
	{
	public:

		struct FInitConfig
		{
			bool operator==(const FInitConfig& InOther) const
			{
				return HeapType == InOther.HeapType && 
					HeapFlags && InOther.HeapFlags && 
					ResourceFlags == InOther.ResourceFlags && 
					InitialResourceState == InOther.InitialResourceState;
			}

			D3D12_HEAP_TYPE HeapType;
			D3D12_HEAP_FLAGS HeapFlags;
			D3D12_RESOURCE_FLAGS ResourceFlags;
			D3D12_RESOURCE_STATES InitialResourceState;
		};

		RenderResourceAllocator(class Device* ParentDevice, const FInitConfig& InInitConfig, const std::string& Name, uint32 MaxSizeForPooling);
		~RenderResourceAllocator();

		const FInitConfig& GetInitConfig() const { return InitConfig; }
		const uint32 GetMaximumAllocationSizeForPooling() const { return MaximumAllocationSizeForPooling; }

	protected:

		const FInitConfig InitConfig;
		const std::string DebugName;
		bool Initialized;

		const uint32 MaximumAllocationSizeForPooling;

		CriticalSection CS;
	};

	class RenderBuddyAllocator : public RenderResourceAllocator
	{
	public:

		enum class EAllocationStrategy
		{
			// This strategy uses Placed Resources to sub-allocate a buffer out of an underlying ID3D12Heap.
			// The benefit of this is that each buffer can have it's own resource state and can be treated
			// as any other buffer. The downside of this strategy is the API limitation which enforces
			// the minimum buffer size to 64k leading to large internal fragmentation in the allocator
			kPlacedResource,
			// The alternative is to manually sub-allocate out of a single large buffer which allows block
			// allocation granularity down to 1 byte. However, this strategy is only really valid for buffers which
			// will be treated as read-only after their creation (i.e. most Index and Vertex buffers). This 
			// is because the underlying resource can only have one state at a time.
			kManualSubAllocation
		};

		RenderBuddyAllocator( Device* ParentDevice, const FInitConfig& InInitConfig, const std::string& Name,
			EAllocationStrategy InAllocationStrategy, uint32 MaxSizeForPooling, uint32 InMaxBlockSize, uint32 InMinBlockSize);

		bool TryAllocate(uint32 SizeInBytes, uint32 Alignment, ResourceLocation& ResourceLocation);

		void Deallocate(ResourceLocation& ResourceLocation);

		void Initialize();

		void Destroy();

		void CleanUpAllocations();

		//void DumpAllocatorStats(class FOutputDevice& Ar);
		//void UpdateMemoryStats(uint32& IOMemoryAllocated, uint32& IOMemoryUsed, uint32& IOMemoryFree, uint32& IOAlignmentWaste, uint32& IOAllocatedPageCount, uint32& IOFullPageCount);

		void ReleaseAllResources();

		void Reset();

		inline bool IsEmpty()
		{
			return FreeBlocks[MaxOrder].size() == 1;
		}
		inline uint64 GetLastUsedFrameFence() const { return LastUsedFrameFence; }

		inline uint32 GetTotalSizeUsed() const { return TotalSizeUsed; }
		inline uint64 GetAllocationOffsetInBytes(const BuddyAllocatorData& AllocatorPrivateData) const { return uint64(AllocatorPrivateData.Offset) * MinBlockSize; }

		//inline FD3D12Heap* GetBackingHeap() { drn_check(AllocationStrategy == EAllocationStrategy::kPlacedResource); return BackingHeap.GetReference(); }

		inline bool IsOwner(ResourceLocation& ResourceLocation)
		{
			return ResourceLocation.GetAllocator() == this;
		}

	protected:

		const uint32 MaxBlockSize;
		const uint32 MinBlockSize;
		const RenderBuddyAllocator::EAllocationStrategy AllocationStrategy;

		TRefCountPtr<RenderResource> BackingResource;
		//TRefCountPtr<FD3D12Heap> BackingHeap;

	private:
		struct RetiredBlock
		{
			RenderResource* PlacedResource;
			uint64 FrameFence;
			BuddyAllocatorData Data;
		};

		std::vector<RetiredBlock> DeferredDeletionQueue;
		std::vector<std::set<uint32>> FreeBlocks;
		uint64 LastUsedFrameFence;
		uint32 MaxOrder;
		uint32 TotalSizeUsed;

		bool HeapFullMessageDisplayed;

		inline uint32 SizeToUnitSize(uint32 size) const
		{
			return (size + (MinBlockSize - 1)) / MinBlockSize;
		}

		inline uint32 UnitSizeToOrder(uint32 size) const
		{
			unsigned long Result;
			_BitScanReverse(&Result, size + size - 1); // ceil(log2(size))
			return Result;
		}

		inline uint32 GetBuddyOffset(const uint32 &offset, const uint32 &size)
		{
			return offset ^ size;
		}

		uint32 OrderToUnitSize(uint32 order) const { return ((uint32)1) << order; }
		uint32 AllocateBlock(uint32 order);
		void DeallocateBlock(uint32 offset, uint32 order);

		bool CanAllocate(uint32 size, uint32 alignment);

		void DeallocateInternal(RetiredBlock& Block);

		void Allocate(uint32 SizeInBytes, uint32 Alignment, ResourceLocation& ResourceLocation);
	};

	class RenderMultiBuddyAllocator : public RenderResourceAllocator
	{
	public:

		RenderMultiBuddyAllocator( class Device* ParentDevice,
			const FInitConfig& InInitConfig,
			const std::string& Name,
			RenderBuddyAllocator::EAllocationStrategy InAllocationStrategy,
			uint32 InMaxAllocationSize,
			uint32 InDefaultPoolSize,
			uint32 InMinBlockSize);
		~RenderMultiBuddyAllocator();

		const RenderBuddyAllocator::EAllocationStrategy GetAllocationStrategy() const { return AllocationStrategy; }

		bool TryAllocate(uint32 SizeInBytes, uint32 Alignment, ResourceLocation& ResourceLocation);

		void Deallocate(ResourceLocation& ResourceLocation);

		void Initialize();

		void Destroy();

		void CleanUpAllocations(uint64 InFrameLag);

		//void DumpAllocatorStats(class FOutputDevice& Ar);
		//void UpdateMemoryStats(uint32& IOMemoryAllocated, uint32& IOMemoryUsed, uint32& IOMemoryFree, uint32& IOAlignmentWaste, uint32& IOAllocatedPageCount, uint32& IOFullPageCount);

		void ReleaseAllResources();

		void Reset();
	
	protected:

		RenderBuddyAllocator* CreateNewAllocator(uint32 InMinSizeInBytes);

		const RenderBuddyAllocator::EAllocationStrategy AllocationStrategy;
		const uint32 MinBlockSize;
		const uint32 DefaultPoolSize;

		std::vector<RenderBuddyAllocator*> Allocators;
	};

	class DefaultBufferPool : public DeviceChild
	{
	public:
		DefaultBufferPool(Device* InParent, RenderMultiBuddyAllocator* InAllocator);
		~DefaultBufferPool() { delete Allocator; }

		bool SupportsAllocation(D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags, EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking) const;
		void AllocDefaultResource(D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& Desc, EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking, ResourceLocation& ResourceLocation, uint32 Alignment, const std::string& Name);
		void CleanUpAllocations(uint64 InFrameLag);
		//void UpdateMemoryStats(uint32& IOMemoryAllocated, uint32& IOMemoryUsed, uint32& IOMemoryFree, uint32& IOAlignmentWaste, uint32& IOAllocatedPageCount, uint32& IOFullPageCount);

		static RenderResourceAllocator::FInitConfig GetResourceAllocatorInitConfig(D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags, EBufferUsageFlags InBufferUsage);
		static RenderBuddyAllocator::EAllocationStrategy GetBuddyAllocatorAllocationStrategy(D3D12_RESOURCE_FLAGS InResourceFlags, bool bNeedsStateTracking);

	private:
		RenderMultiBuddyAllocator* Allocator;
	};

	class DefaultBufferAllocator : public DeviceChild
	{
	public:
		DefaultBufferAllocator(class Device* InParent);
	
		void AllocDefaultResource(D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& pDesc, EBufferUsageFlags InBufferUsage,
			bool bNeedsStateTracking, ResourceLocation& ResourceLocation, uint32 Alignment, const std::string& Name);
		void FreeDefaultBufferPools();
		void CleanupFreeBlocks(uint64 InFrameLag);
		void UpdateMemoryStats();
	
	private:
	
		DefaultBufferPool* CreateBufferPool(D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags, EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking);
	
		std::vector<DefaultBufferPool*> DefaultBufferPools;
	};


}