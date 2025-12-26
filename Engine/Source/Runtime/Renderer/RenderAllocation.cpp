#include "DrnPCH.h"
#include "RenderAllocation.h"

#define FAST_ALLOCATOR_MIN_PAGES_TO_RETAIN 5

namespace Drn
{
	RenderResourceAllocator::RenderResourceAllocator( class Device* ParentDevice, const FInitConfig& InInitConfig, const std::string& Name, uint32 MaxSizeForPooling )
		: DeviceChild(ParentDevice)
		, InitConfig(InInitConfig)
		, DebugName(Name)
		, MaximumAllocationSizeForPooling(MaxSizeForPooling)
		, Initialized(false)
	{}

	RenderResourceAllocator::~RenderResourceAllocator()
	{}

// -------------------------------------------------------------------------------------------------------------------------

	RenderBuddyAllocator::RenderBuddyAllocator( Device* ParentDevice, const FInitConfig& InInitConfig, const std::string& Name, EAllocationStrategy InAllocationStrategy,
		uint32 MaxSizeForPooling, uint32 InMaxBlockSize, uint32 InMinBlockSize )
		: RenderResourceAllocator(ParentDevice, InInitConfig, Name, MaxSizeForPooling)
		, MaxBlockSize(InMaxBlockSize)
		, MinBlockSize(InMinBlockSize)
		, AllocationStrategy(InAllocationStrategy)
		, LastUsedFrameFence(0)
		, TotalSizeUsed(0)
		, HeapFullMessageDisplayed(false)
	{
		// maxBlockSize should be evenly dividable by MinBlockSize and  
		// maxBlockSize / MinBlockSize should be a power of two  
		drn_check((MaxBlockSize / MinBlockSize) * MinBlockSize == MaxBlockSize); // Evenly dividable  
		drn_check(0 == ((MaxBlockSize / MinBlockSize) & ((MaxBlockSize / MinBlockSize) - 1))); // Power of two  

		MaxOrder = UnitSizeToOrder(SizeToUnitSize(MaxBlockSize));

		Reset();
	}

	bool RenderBuddyAllocator::TryAllocate( uint32 SizeInBytes, uint32 Alignment, ResourceLocation& ResourceLocation )
	{
		ScopeLock Lock(&CS);

		if (CanAllocate(SizeInBytes, Alignment))
		{
			Allocate(SizeInBytes, Alignment, ResourceLocation);
			return true;
		}
		else
		{
			return false;
		}
	}

	void RenderBuddyAllocator::Deallocate( ResourceLocation& ResourceLocation )
	{
		drn_check(IsOwner(ResourceLocation));
		ScopeLock Lock(&CS);

		GpuFence& FrameFence = *Renderer::Get()->GetFence();

		DeferredDeletionQueue.push_back({});
		RetiredBlock& Block = DeferredDeletionQueue.back();
		Block.FrameFence = FrameFence.GetCurrentFence();
		BuddyAllocatorData& Data = ResourceLocation.GetBuddyAllocatorData();
		Block.Data.Order = Data.Order;
		Block.Data.Offset = Data.Offset;

		// update the last used framce fence used during garbage collection
		LastUsedFrameFence = std::max(LastUsedFrameFence, Block.FrameFence);

		//if (ResourceLocation.GetResource()->IsPlacedResource())
		//{
		//	Block.PlacedResource = ResourceLocation.GetResource();
		//}
	}

	void RenderBuddyAllocator::Initialize()
	{
		Device* Device = GetParentDevice();

//		if (AllocationStrategy == EAllocationStrategy::kPlacedResource)
//		{
//			D3D12_HEAP_PROPERTIES HeapProps = CD3DX12_HEAP_PROPERTIES(InitConfig.HeapType);
//			HeapProps.CreationNodeMask = GetGPUMask().GetNative();
//			HeapProps.VisibleNodeMask = GetVisibilityMask().GetNative();
//
//			D3D12_HEAP_DESC Desc = {};
//			Desc.SizeInBytes = MaxBlockSize;
//			Desc.Properties = HeapProps;
//			Desc.Alignment = 0;
//			Desc.Flags = InitConfig.HeapFlags;
//#if PLATFORM_WINDOWS
//			if (Adapter->IsHeapNotZeroedSupported())
//			{
//				Desc.Flags |= D3D12_HEAP_FLAG_CREATE_NOT_ZEROED;
//			}
//#endif
//
//			ID3D12Heap* Heap = nullptr;
//			{
//				LLM_PLATFORM_SCOPE(ELLMTag::GraphicsPlatform);
//
//				// we are tracking allocations ourselves, so don't let XMemAlloc track these as well
//				LLM_SCOPED_PAUSE_TRACKING_FOR_TRACKER(ELLMTracker::Default, ELLMAllocType::System);
//				VERIFYD3D12RESULT(Adapter->GetD3DDevice()->CreateHeap(&Desc, IID_PPV_ARGS(&Heap)));
//			}
//			SetName(Heap, L"Placed Resource Allocator Backing Heap");
//
//			BackingHeap = new FD3D12Heap(GetParentDevice(), GetVisibilityMask());
//			BackingHeap->SetHeap(Heap);
//
//			// Only track resources that cannot be accessed on the CPU.
//			if (IsCPUInaccessible(InitConfig.HeapType))
//			{
//				BackingHeap->BeginTrackingResidency(Desc.SizeInBytes);
//			}
//		}
//		else
		{
			// https://stackoverflow.com/questions/77450713/why-am-i-getting-an-unhandled-exception-in-kernelbase-dll-when-im-trying-to-cal
			Device->CreateBuffer(InitConfig.HeapType, MaxBlockSize, InitConfig.InitialResourceState, false,
				BackingResource.GetInitReference(), "Resource Allocator Underlying Buffer", InitConfig.ResourceFlags);

			if (IsCPUWritable(InitConfig.HeapType))
			{
				BackingResource->Map();
			}
		}
	}

	void RenderBuddyAllocator::Destroy()
	{
		ReleaseAllResources();
	}

	void RenderBuddyAllocator::CleanUpAllocations()
	{
		ScopeLock Lock(&CS);

		GpuFence& FrameFence = *Renderer::Get()->GetFence();

		uint32 PopCount = 0;
		for (int32 i = 0; i < DeferredDeletionQueue.size(); i++)
		{
			RetiredBlock& Block = DeferredDeletionQueue[i];

			if (FrameFence.IsFenceComplete(Block.FrameFence))
			{
				DeallocateInternal(Block);
				PopCount = i + 1;
			}
			else
			{
				break;
			}
		}

		if (PopCount)
		{
			DeferredDeletionQueue.erase(DeferredDeletionQueue.begin(), DeferredDeletionQueue.begin() + PopCount);
		}
	}

	void RenderBuddyAllocator::ReleaseAllResources()
	{
		for (RetiredBlock& Block : DeferredDeletionQueue)
		{
			DeallocateInternal(Block);
		}

		DeferredDeletionQueue.clear();

		if (BackingResource)
		{
			drn_check(BackingResource->GetRefCount() == 1);
			BackingResource = nullptr;
		}

		//if (BackingHeap)
		//{
		//	BackingHeap->Destroy();
		//}
	}

	void RenderBuddyAllocator::Reset()
	{
		FreeBlocks.clear();

		FreeBlocks.resize(MaxOrder + 1);
		FreeBlocks[MaxOrder].insert((uint32)0);
	}

	uint32 RenderBuddyAllocator::AllocateBlock( uint32 order )
	{
		uint32 offset;

		if (order > MaxOrder)
		{
			drn_check(false);
		}

		if (FreeBlocks[order].size() == 0)
		{
			uint32 left = AllocateBlock(order + 1);
			uint32 size = OrderToUnitSize(order);
			uint32 right = left + size;
			FreeBlocks[order].insert(right);
			offset = left;
		}

		else
		{
			std::set<uint32>::const_iterator it = FreeBlocks[order].begin();
			offset = *it;
			FreeBlocks[order].erase(*it);
		}

		return offset;
	}

	void RenderBuddyAllocator::DeallocateBlock( uint32 offset, uint32 order )
	{
		uint32 size = OrderToUnitSize(order);
		uint32 buddy = GetBuddyOffset(offset, size);

		auto it = FreeBlocks[order].find(buddy);

		if (it != FreeBlocks[order].end())
		{
			DeallocateBlock(std::min(offset, buddy), order + 1);
			FreeBlocks[order].erase(*it);
		}
		else
		{
			FreeBlocks[order].insert(offset);
		}
	}

	bool RenderBuddyAllocator::CanAllocate( uint32 size, uint32 alignment )
	{
		if (TotalSizeUsed == MaxBlockSize)
		{
			return false;
		}

		uint32 sizeToAllocate = size;
		// If the alignment doesn't match the block size
		if (alignment != 0 && MinBlockSize % alignment != 0)
		{
			sizeToAllocate = size + alignment;
		}

		uint32 blockSize = MaxBlockSize;

		for (int32 i = FreeBlocks.size() - 1; i >= 0; i--)
		{
			if (FreeBlocks[i].size() && blockSize >= sizeToAllocate)
			{
				return true;
			}

			// Halve the block size;
			blockSize = blockSize >> 1;

			if (blockSize < sizeToAllocate) return false;
		}
		return false;
	}

	void RenderBuddyAllocator::DeallocateInternal( RetiredBlock& Block )
	{
		DeallocateBlock(Block.Data.Offset, Block.Data.Order);

		const uint32 Size = uint32(OrderToUnitSize(Block.Data.Order) * MinBlockSize);
		TotalSizeUsed -= Size;

		if (AllocationStrategy == EAllocationStrategy::kPlacedResource)
		{
			// Release the resource
			drn_check(Block.PlacedResource != nullptr);
			Block.PlacedResource->Release();
			Block.PlacedResource = nullptr;
		}
	}

	void RenderBuddyAllocator::Allocate( uint32 SizeInBytes, uint32 Alignment, ResourceLocation& ResourceLocation )
	{
		ScopeLock Lock(&CS);

		if (Initialized == false)
		{
			Initialize();
			Initialized = true;
		}

		uint32 SizeToAllocate = SizeInBytes;

		// If the alignment doesn't match the block size
		if (Alignment != 0 && MinBlockSize % Alignment != 0)
		{
			SizeToAllocate = SizeInBytes + Alignment;
		}

		const uint32 UnitSize = SizeToUnitSize(SizeToAllocate);
		const uint32 Order = UnitSizeToOrder(UnitSize);
		const uint32 Offset = AllocateBlock(Order);

		const uint32 AllocSize = uint32(OrderToUnitSize(Order) * MinBlockSize);
		const uint32 AllocationBlockOffset = uint32(Offset * MinBlockSize);
		uint32 Padding = 0;

		if (Alignment != 0 && AllocationBlockOffset % Alignment != 0)
		{
			uint32 AlignedBlockOffset = AlignArbitrary(AllocationBlockOffset, Alignment);
			Padding = AlignedBlockOffset - AllocationBlockOffset;

			drn_check((Padding + SizeInBytes) <= AllocSize)
		}
	
		TotalSizeUsed += AllocSize;

		const uint32 AlignedOffsetFromResourceBase = AllocationBlockOffset + Padding;
		drn_check((AlignedOffsetFromResourceBase % Alignment) == 0);

		BuddyAllocatorData& Data = ResourceLocation.GetBuddyAllocatorData();
		Data.Order = Order;
		Data.Offset = Offset;

		ResourceLocation.SetType(ResourceLocation::ResourceLocationType::eSubAllocation);
		ResourceLocation.SetAllocator(this);
		ResourceLocation.SetSize(SizeInBytes);

		if (AllocationStrategy == EAllocationStrategy::kManualSubAllocation)
		{
			ResourceLocation.SetOffsetFromBaseOfResource(AlignedOffsetFromResourceBase);
			ResourceLocation.SetResource(BackingResource);
			ResourceLocation.SetGPUVirtualAddress(BackingResource->GetGPUVirtualAddress() + AlignedOffsetFromResourceBase);

			if (IsCPUWritable(InitConfig.HeapType))
			{
				ResourceLocation.SetMappedBaseAddress((uint8*)BackingResource->GetResourceBaseAddress() + AlignedOffsetFromResourceBase);
			}
		}
		else
		{
			// Place resources are intialized elsewhere
		}
	}

// -------------------------------------------------------------------------------------------------------------------------

	RenderMultiBuddyAllocator::RenderMultiBuddyAllocator( Device* ParentDevice, const FInitConfig& InInitConfig, const std::string& Name,
		RenderBuddyAllocator::EAllocationStrategy InAllocationStrategy, uint32 InMaxAllocationSize, uint32 InDefaultPoolSize, uint32 InMinBlockSize )
		: RenderResourceAllocator(ParentDevice, InInitConfig, Name, InMaxAllocationSize)
		, AllocationStrategy(InAllocationStrategy)
		, MinBlockSize(InMinBlockSize)
		, DefaultPoolSize(InDefaultPoolSize)
	{}

	RenderMultiBuddyAllocator::~RenderMultiBuddyAllocator()
	{
		Destroy();
	}

	bool RenderMultiBuddyAllocator::TryAllocate( uint32 SizeInBytes, uint32 Alignment, ResourceLocation& ResourceLocation )
	{
		ScopeLock Lock(&CS);

		for (int32 i = 0; i < Allocators.size(); i++)
		{
			if (Allocators[i]->TryAllocate(SizeInBytes, Alignment, ResourceLocation))
			{
				return true;
			}
		}

		Allocators.push_back(CreateNewAllocator(SizeInBytes));
		return Allocators.back()->TryAllocate(SizeInBytes, Alignment, ResourceLocation);
	}

	void RenderMultiBuddyAllocator::Deallocate( ResourceLocation& ResourceLocation )
	{
		drn_check(false);
	}

	void RenderMultiBuddyAllocator::Initialize()
	{
		Allocators.push_back(CreateNewAllocator(DefaultPoolSize));
	}

	void RenderMultiBuddyAllocator::Destroy()
	{
		ReleaseAllResources();
	}

	void RenderMultiBuddyAllocator::CleanUpAllocations( uint64 InFrameLag )
	{
		ScopeLock Lock(&CS);

		for (auto*& Allocator : Allocators)
		{
			Allocator->CleanUpAllocations();
		}

		GpuFence& FrameFence = *Renderer::Get()->GetFence();
		const uint64 CompletedFence = FrameFence.UpdateCompletedFence();

		for (int32 i = (Allocators.size() - 1); i >= 0; i--)
		{
			if (Allocators[i]->IsEmpty() && (Allocators[i]->GetLastUsedFrameFence() + InFrameLag <= CompletedFence))
			{
				Allocators[i]->Destroy();
				delete(Allocators[i]);
				Allocators.erase(Allocators.begin() + i);
			}
		}
	}

	void RenderMultiBuddyAllocator::ReleaseAllResources()
	{
		for (int32 i = (Allocators.size() - 1); i >= 0; i--)
		{
			Allocators[i]->Destroy();
			delete(Allocators[i]);
		}

		Allocators.clear();
	}

	void RenderMultiBuddyAllocator::Reset()
	{
		
	}

	RenderBuddyAllocator* RenderMultiBuddyAllocator::CreateNewAllocator( uint32 InMinSizeInBytes )
	{
		drn_check(InMinSizeInBytes <= MaximumAllocationSizeForPooling);
		uint32 AllocationSize = (InMinSizeInBytes > DefaultPoolSize) ? Math::RoundUpToPowerOfTwo(InMinSizeInBytes) : DefaultPoolSize;

		return new RenderBuddyAllocator(GetParentDevice(), InitConfig, DebugName, AllocationStrategy, AllocationSize, AllocationSize, MinBlockSize);
	}

// -------------------------------------------------------------------------------------------------------------------------

	RenderResourceAllocator::FInitConfig DefaultBufferPool::GetResourceAllocatorInitConfig(
		D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags, EBufferUsageFlags InBufferUsage )
	{
		RenderResourceAllocator::FInitConfig InitConfig;
		InitConfig.HeapType = InHeapType;
		InitConfig.ResourceFlags = InResourceFlags;

//#if D3D12_RHI_RAYTRACING
//		// Setup initial resource state depending on the requested buffer flags
//		if (EnumHasAnyFlags(InBufferUsage, BUF_AccelerationStructure))
//		{
//			// should only have this flag and no other flags
//			check(InBufferUsage == BUF_AccelerationStructure);
//			InitConfig.InitialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
//		}
//		else 
//#endif // D3D12_RHI_RAYTRACING
		if (InitConfig.HeapType == D3D12_HEAP_TYPE_READBACK)
		{
			InitConfig.InitialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else if (EnumHasAnyFlags(InBufferUsage, EBufferUsageFlags::UnorderedAccess))
		{
			drn_check(InResourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			InitConfig.InitialResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}
		else
		{
			InitConfig.InitialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}

		InitConfig.HeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		if (EnumHasAnyFlags(InBufferUsage, EBufferUsageFlags::DrawIndirect))
		{
			drn_check(InResourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			//InitConfig.HeapFlags |= D3D12RHI_HEAP_FLAG_ALLOW_INDIRECT_BUFFERS;
		}

		return InitConfig;
	}

	RenderBuddyAllocator::EAllocationStrategy DefaultBufferPool::GetBuddyAllocatorAllocationStrategy( D3D12_RESOURCE_FLAGS InResourceFlags, bool bNeedsStateTracking )
	{
		// Does the resource need state tracking and transitions
		//ED3D12ResourceStateMode ResourceStateMode = InResourceStateMode;
		//if (ResourceStateMode == ED3D12ResourceStateMode::Default)
		//{
		//	ResourceStateMode = (InResourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) ? ED3D12ResourceStateMode::MultiState : ED3D12ResourceStateMode::SingleState;
		//}
		//
		//// multi state resource need to placed because each allocation can be in a different state
		//return (ResourceStateMode == ED3D12ResourceStateMode::MultiState) ?	FD3D12BuddyAllocator::EAllocationStrategy::kPlacedResource : FD3D12BuddyAllocator::EAllocationStrategy::kManualSubAllocation;

		drn_check(!bNeedsStateTracking);
		return bNeedsStateTracking ? RenderBuddyAllocator::EAllocationStrategy::kPlacedResource : RenderBuddyAllocator::EAllocationStrategy::kManualSubAllocation;
	}

	DefaultBufferPool::DefaultBufferPool( Device* InParent, RenderMultiBuddyAllocator* InAllocator )
		: DeviceChild(InParent)
		, Allocator(InAllocator)
	{}

	bool DefaultBufferPool::SupportsAllocation( D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags, EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking ) const
	{
		RenderResourceAllocator::FInitConfig InitConfig = GetResourceAllocatorInitConfig(InHeapType, InResourceFlags, InBufferUsage);

		RenderBuddyAllocator::EAllocationStrategy AllocationStrategy = GetBuddyAllocatorAllocationStrategy(InResourceFlags, bNeedsStateTracking);
		return (Allocator->GetInitConfig() == InitConfig && Allocator->GetAllocationStrategy() == AllocationStrategy);
	}

	void DefaultBufferPool::AllocDefaultResource( D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& Desc, EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking,
		ResourceLocation& ResourceLocation, uint32 Alignment, const std::string& Name )
	{
		Device* Device = GetParentDevice();

		ResourceLocation.Clear();

		if (Desc.Width == 0)
		{
			return;
		}

		D3D12_RESOURCE_STATES InitialState = D3D12_RESOURCE_STATE_COMMON;

//#if D3D12_RHI_RAYTRACING
//		if (InUsage & BUF_AccelerationStructure)
//		{
//			// RayTracing acceleration structures must be created in a particular state and may never transition out of it.
//			check(InResourceStateMode == ED3D12ResourceStateMode::SingleState);
//		}
//
//		if (InResourceStateMode == ED3D12ResourceStateMode::SingleState)
//		{
//			if (InUsage & BUF_UnorderedAccess)
//			{
//				check((InUsage & ~BUF_UnorderedAccess) == 0);
//				InitialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
//			}
//
//			if (InUsage & BUF_AccelerationStructure)
//			{
//				check((InUsage & ~BUF_AccelerationStructure) == 0);
//				InitialState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
//			}
//		}
//#endif

		if (InHeapType == D3D12_HEAP_TYPE_READBACK)
		{
			InitialState = D3D12_RESOURCE_STATE_COPY_DEST;
		}

		const bool PoolResource = Desc.Width < Allocator->GetMaximumAllocationSizeForPooling()/* && ((Desc.Width % (1024 * 64)) != 0)*/;

		if (PoolResource)
		{
			const bool bPlacedResource = (Allocator->GetAllocationStrategy() == RenderBuddyAllocator::EAllocationStrategy::kPlacedResource);

			//if (bPlacedResource)
			//{
			//	// Writeable resources get separate ID3D12Resource* with their own resource state by using placed resources. Just make sure it's UAV, other flags are free to differ.
			//	check((Desc.Flags & Allocator->GetInitConfig().ResourceFlags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0 || InHeapType == D3D12_HEAP_TYPE_READBACK);
			//}
			//else
			{
				// Read-only resources get suballocated from big resources, thus share ID3D12Resource* and resource state with other resources. Ensure it's suballocated from a resource with identical flags.
				drn_check(Desc.Flags == Allocator->GetInitConfig().ResourceFlags);
			}
	
			if (Allocator->TryAllocate(Desc.Width, Alignment, ResourceLocation))
			{
				//if (bPlacedResource)
				//{
				//	drn_check(ResourceLocation.GetResource() == nullptr);
				//
				//	FD3D12Heap* BackingHeap = ((FD3D12BuddyAllocator*) ResourceLocation.GetAllocator())->GetBackingHeap();
				//	uint64 HeapOffset = ResourceLocation.GetAllocator()->GetAllocationOffsetInBytes(ResourceLocation.GetBuddyAllocatorPrivateData());
				//
				//	FD3D12Resource* NewResource = nullptr;
				//	VERIFYD3D12RESULT(Adapter->CreatePlacedResource(Desc, BackingHeap, HeapOffset, InitialState, ED3D12ResourceStateMode::MultiState, D3D12_RESOURCE_STATE_TBD, nullptr, &NewResource, Name));
				//
				//	ResourceLocation.SetResource(NewResource);
				//}
				//else
				{
					// Nothing to do for suballocated resources
				}

				// Successfully sub-allocated
				return;
			}
		}

		// TODO: remove. for now disable for testing
		//drn_check(false);

		RenderResource* NewResource = nullptr;
		Device->CreateBuffer(InHeapType, Desc.Width, InitialState, bNeedsStateTracking, &NewResource, Name, Desc.Flags);

		ResourceLocation.AsStandAlone(NewResource, Desc.Width);
	}

	void DefaultBufferPool::CleanUpAllocations( uint64 InFrameLag )
	{
		Allocator->CleanUpAllocations(InFrameLag);
	}

// -------------------------------------------------------------------------------------------------------------------------

	DefaultBufferAllocator::DefaultBufferAllocator( Device* InParent )
		: DeviceChild(InParent)
	{}

	void DefaultBufferAllocator::AllocDefaultResource( D3D12_HEAP_TYPE InHeapType, const D3D12_RESOURCE_DESC& pDesc, EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking,
		ResourceLocation& ResourceLocation, uint32 Alignment, const std::string& Name )
	{
		D3D12_RESOURCE_DESC ResourceDesc = pDesc;
		ResourceDesc.Flags = ResourceDesc.Flags & (~D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

		DefaultBufferPool* BufferPool = nullptr;
		for (DefaultBufferPool* Pool : DefaultBufferPools)
		{
			if (Pool->SupportsAllocation(InHeapType, ResourceDesc.Flags, InBufferUsage, bNeedsStateTracking))
			{
				BufferPool = Pool;
				break;
			}
		}

		if (BufferPool == nullptr)
		{
			BufferPool = CreateBufferPool(InHeapType, ResourceDesc.Flags, InBufferUsage, bNeedsStateTracking);
		}

		BufferPool->AllocDefaultResource(InHeapType, ResourceDesc, InBufferUsage, bNeedsStateTracking, ResourceLocation, Alignment, Name);
	}

	void DefaultBufferAllocator::FreeDefaultBufferPools()
	{
		for (DefaultBufferPool*& DefaultBufferPool : DefaultBufferPools)
		{
			if (DefaultBufferPool)
			{
				DefaultBufferPool->CleanUpAllocations(0);

				delete DefaultBufferPool;
				DefaultBufferPool = nullptr;
			}
		}
	}

	void DefaultBufferAllocator::CleanupFreeBlocks( uint64 InFrameLag )
	{
		SCOPE_STAT();

		for (DefaultBufferPool* DefaultBufferPool : DefaultBufferPools)
		{
			if (DefaultBufferPool)
			{
				DefaultBufferPool->CleanUpAllocations(InFrameLag);
			}
		}
	}

	void DefaultBufferAllocator::UpdateMemoryStats()
	{
		
	}

	DefaultBufferPool* DefaultBufferAllocator::CreateBufferPool( D3D12_HEAP_TYPE InHeapType, D3D12_RESOURCE_FLAGS InResourceFlags,
		EBufferUsageFlags InBufferUsage, bool bNeedsStateTracking )
	{
		Device* Device = GetParentDevice();
		RenderMultiBuddyAllocator* Allocator = nullptr;

		RenderResourceAllocator::FInitConfig InitConfig = DefaultBufferPool::GetResourceAllocatorInitConfig(InHeapType, InResourceFlags, InBufferUsage);

		RenderBuddyAllocator::EAllocationStrategy AllocationStrategy = DefaultBufferPool::GetBuddyAllocatorAllocationStrategy(InResourceFlags, bNeedsStateTracking);

		uint32 MinBlockSize = (AllocationStrategy == RenderBuddyAllocator::EAllocationStrategy::kPlacedResource) ? MIN_PLACED_BUFFER_SIZE : 16;

		const std::string Name("Default Buffer Multi Buddy Allocator");
		Allocator = new RenderMultiBuddyAllocator(Device, InitConfig, Name, AllocationStrategy,
			InHeapType == D3D12_HEAP_TYPE_READBACK ? READBACK_BUFFER_POOL_MAX_ALLOC_SIZE : DEFAULT_BUFFER_POOL_MAX_ALLOC_SIZE,
			InHeapType == D3D12_HEAP_TYPE_READBACK ? READBACK_BUFFER_POOL_DEFAULT_POOL_SIZE : DEFAULT_BUFFER_POOL_DEFAULT_POOL_SIZE,
			MinBlockSize
			);

		DefaultBufferPool* NewPool = new DefaultBufferPool(Device, Allocator);
		DefaultBufferPools.push_back(NewPool);
		return NewPool;
	}

// -------------------------------------------------------------------------------------------------------------------------

	void FastAllocatorPage::UpdateFence()
	{
		FrameFence = std::max(FrameFence, Renderer::Get()->GetFence()->GetCurrentFence());
	}

	FastAllocatorPagePool::FastAllocatorPagePool( Device* Parent, D3D12_HEAP_TYPE InHeapType, uint32 Size )
		: DeviceChild(Parent)
		, PageSize(Size)
		, HeapProperties(CD3DX12_HEAP_PROPERTIES(InHeapType))
	{}

	FastAllocatorPage* FastAllocatorPagePool::RequestFastAllocatorPage()
	{
		Device* Device = GetParentDevice();
		GpuFence& Fence = *Renderer::Get()->GetFence();

		const uint64 CompletedFence = Fence.UpdateCompletedFence();

		for (int32 Index = 0; Index < Pool.size(); Index++)
		{
			FastAllocatorPage* Page = Pool[Index];

			if (Page->FastAllocBuffer->GetRefCount() == 1 &&
				Page->FrameFence <= CompletedFence)
			{
				Page->Reset();
				Pool.erase(Pool.begin() + Index);
				return Page;
			}
		}

		FastAllocatorPage* Page = new FastAllocatorPage(PageSize);

		const D3D12_RESOURCE_STATES InitialState = DetermineInitialResourceState(HeapProperties.Type, &HeapProperties);
		Device->CreateBuffer(HeapProperties.Type, PageSize, InitialState, false, Page->FastAllocBuffer.GetInitReference(), "Fast Allocator Page", D3D12_RESOURCE_FLAG_NONE);
		Page->FastAllocBuffer->DoNotDeferDelete();

		Page->FastAllocData = Page->FastAllocBuffer->Map();

		return Page;
	}

	void FastAllocatorPagePool::ReturnFastAllocatorPage( FastAllocatorPage* Page )
	{
		Page->UpdateFence();
		Pool.push_back(Page);
	}

	void FastAllocatorPagePool::CleanupPages( uint64 FrameLag )
	{
		if (Pool.size() <= FAST_ALLOCATOR_MIN_PAGES_TO_RETAIN)
		{
			return;
		}

		GpuFence& FrameFence = *Renderer::Get()->GetFence();

		const uint64 CompletedFence = FrameFence.UpdateCompletedFence();

		for (int32 Index = 0; Index < Pool.size(); Index++)
		{
			FastAllocatorPage* Page = Pool[Index];

			if (Page->FastAllocBuffer->GetRefCount() == 1 &&
				Page->FrameFence + FrameLag <= CompletedFence)
			{
				Pool.erase(Pool.begin() + Index);
				delete(Page);

				return;
			}
		}
	}

	void FastAllocatorPagePool::Destroy()
	{
		for (int32 i = 0; i < Pool.size(); i++)
		{
			{
				FastAllocatorPage *Page = Pool[i];
				delete(Page);
				Page = nullptr;
			}
		}

		Pool.clear();
	}


	FastAllocator::FastAllocator( class Device* Parent, D3D12_HEAP_TYPE InHeapType, uint32 PageSize )
		: DeviceChild(Parent)
		, PagePool(Parent, InHeapType, PageSize)
		, CurrentAllocatorPage(nullptr)
	{}

	void* FastAllocator::Allocate( uint32 Size, uint32 Alignment, class ResourceLocation* ResourceLocation )
	{
		drn_check(!ResourceLocation->IsValid());

		if (Size > PagePool.GetPageSize())
		{
			//Allocations are 64k aligned
			if (Alignment)
			{
				Alignment = (D3D_BUFFER_ALIGNMENT % Alignment) == 0 ? 0 : Alignment;
			}

			RenderResource* Resource = nullptr;
			std::string ResourceName;
#if D3D12_Debug_INFO
			static std::atomic<int64> ID = 0;
			const int64 UniqueID = ID.fetch_add(1);
			ResourceName = std::string("Stand Alone Fast Allocation ") + std::to_string(UniqueID);
#endif
			D3D12_RESOURCE_STATES InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
			GetParentDevice()->CreateBuffer(PagePool.GetHeapType(), Size + Alignment, InitalState, false, &Resource, ResourceName, D3D12_RESOURCE_FLAG_NONE);

			void* Data = nullptr;
			if (PagePool.IsCPUWritable())
			{
				Data = Resource->Map();
			}
			ResourceLocation->AsStandAlone(Resource, Size + Alignment);

			return Data;
		}
		else
		{
			ScopeLock Lock(&CS);

			const uint32 Offset = (CurrentAllocatorPage) ? CurrentAllocatorPage->NextFastAllocOffset : 0;
			uint32 CurrentOffset = AlignArbitrary(Offset, Alignment);

			// See if there is room in the current pool
			if (CurrentAllocatorPage == nullptr || PagePool.GetPageSize() < CurrentOffset + Size)
			{
				if (CurrentAllocatorPage)
				{
					PagePool.ReturnFastAllocatorPage(CurrentAllocatorPage);
				}
				CurrentAllocatorPage = PagePool.RequestFastAllocatorPage();

				CurrentOffset = AlignArbitrary(CurrentAllocatorPage->NextFastAllocOffset, Alignment);
			}

			drn_check(PagePool.GetPageSize() - Size >= CurrentOffset);

			ResourceLocation->AsFastAllocation(CurrentAllocatorPage->FastAllocBuffer.GetReference(),
				Size,
				CurrentAllocatorPage->FastAllocBuffer->GetGPUVirtualAddress(),
				CurrentAllocatorPage->FastAllocData,
				0,
				CurrentOffset);

			CurrentAllocatorPage->NextFastAllocOffset = CurrentOffset + Size;
			CurrentAllocatorPage->UpdateFence();

			drn_check(ResourceLocation->GetMappedBaseAddress());
			return ResourceLocation->GetMappedBaseAddress();
		}
	}

	void FastAllocator::Destroy()
	{
		ScopeLock Lock(&CS);
		if (CurrentAllocatorPage)
		{
			PagePool.ReturnFastAllocatorPage(CurrentAllocatorPage);
			CurrentAllocatorPage = nullptr;
		}
	
		PagePool.Destroy();
	}

	void FastAllocator::CleanupPages( uint64 FrameLag )
	{
		SCOPE_STAT();

		ScopeLock Lock(&CS);
		PagePool.CleanupPages(FrameLag);
	}

	FastConstantAllocator::FastConstantAllocator( Device* Parent )
		: DeviceChild(Parent)
		, UnderlyingResource(Parent)
		, Offset(64 * 1024)
		, PageSize(64 * 1024)
	{
		drn_check(PageSize % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0);
	}

	void* FastConstantAllocator::Allocate( uint32 Bytes, ResourceLocation& OutLocation)
	{
		drn_check(Bytes <= PageSize);

		const uint32 AlignedSize = Align(Bytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

		if (Offset + AlignedSize > PageSize)
		{
			Offset = 0;
			Device* Device = GetParentDevice();

			DynamicHeapAllocator& Allocator = Device->GetDynamicHeapAllocator();
			Allocator.AllocUploadResource(PageSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, UnderlyingResource);
		}

		OutLocation.AsFastAllocation(UnderlyingResource.GetResource(),
			AlignedSize,
			UnderlyingResource.GetGPUVirtualAddress(),
			UnderlyingResource.GetMappedBaseAddress(),
			UnderlyingResource.GetOffsetFromBaseOfResource(),
			Offset);

		Offset += AlignedSize;
		return OutLocation.GetMappedBaseAddress();
	}

	void FastConstantAllocator::Destroy()
	{
		UnderlyingResource.Clear();
	}

// -------------------------------------------------------------------------------------------------------------------------

	DynamicHeapAllocator::DynamicHeapAllocator( class Device* InParent, const std::string& InName, RenderBuddyAllocator::EAllocationStrategy InAllocationStrategy,
		uint32 InMaxSizeForPooling, uint32 InMaxBlockSize, uint32 InMinBlockSize )
		: DeviceChild(InParent)
		, Allocator(InParent, RenderResourceAllocator::FInitConfig{D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ}
		, InName, InAllocationStrategy, InMaxSizeForPooling, InMaxBlockSize, InMinBlockSize)
	{}

	void DynamicHeapAllocator::Init()
	{}

	void* DynamicHeapAllocator::AllocUploadResource( uint32 Size, uint32 Alignment, ResourceLocation& ResourceLocation )
	{
		Device* Device = GetParentDevice();
		ResourceLocation.Clear();

		if (Size == 0)
		{
			Size = 16;
		}
	
		if (Device->GetDeferredDeletionQueue().QueueSize() > 128)
		{
			Device->GetDeferredDeletionQueue().ReleaseResources();
			Allocator.CleanUpAllocations(0);
		}
	
		if (Size <= Allocator.GetMaximumAllocationSizeForPooling())
		{
			if (Allocator.TryAllocate(Size, Alignment, ResourceLocation))
			{
				return ResourceLocation.GetMappedBaseAddress();
			}
		}

		RenderResource* NewResource = nullptr;

		D3D12_RESOURCE_STATES InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
		Device->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD, Size, InitalState, false, &NewResource, "Stand Alone Upload Buffer", D3D12_RESOURCE_FLAG_NONE);

		ResourceLocation.AsStandAlone(NewResource, Size);
		return ResourceLocation.GetMappedBaseAddress();
	}

	void DynamicHeapAllocator::CleanUpAllocations( uint64 InFrameLag )
	{
		SCOPE_STAT();

		Allocator.CleanUpAllocations(InFrameLag);
	}

	void DynamicHeapAllocator::Destroy()
	{
		Allocator.Destroy();
	}



        }  // namespace Drn