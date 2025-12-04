#include "DrnPCH.h"
#include "RenderResource.h"

namespace Drn
{
#if D3D12_Debug_INFO
	std::atomic<int64> RenderResource::TotalResourceCount = 0;
	std::atomic<int64> RenderResource::NoStateTrackingResourceCount = 0;
#endif

	RenderResource::RenderResource( Device* ParentDevice, ID3D12Resource* InResource, D3D12_RESOURCE_DESC const& InDesc,
		D3D12_RESOURCE_STATES InInitialResourceState, bool InNeedsStateTracking, D3D12_HEAP_TYPE InHeapType )
		: DeviceChild(ParentDevice)
		, Resource(InResource)
		, Desc(InDesc)
		, PlaneCount(D3D12GetFormatPlaneCount(ParentDevice->GetD3D12Device(), InDesc.Format))
		, SubresourceCount(0)
		, bRequiresResourceStateTracking(InNeedsStateTracking)
		, bDepthStencil(false)
		, bDeferDelete(true)
		, bBackBuffer(false)
		, HeapType(InHeapType)
		, GPUVirtualAddress(0)
		, ResourceBaseAddress(nullptr)
	{
#if D3D12_Debug_INFO
		TotalResourceCount.fetch_add(1);
#endif

		if (Resource && Desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			GPUVirtualAddress = Resource->GetGPUVirtualAddress();
		}

		SubresourceCount = GetMipLevels() * GetArraySize() * GetPlaneCount();
		if (bRequiresResourceStateTracking)
		{
			ResourceState.Initialize(SubresourceCount);
			ResourceState.SetResourceState(InInitialResourceState);
		}
	}

	RenderResource::~RenderResource()
	{
		
	}

	void* RenderResource::Map( const D3D12_RANGE* ReadRange /*= nullptr*/ )
	{
		if (NumMapCalls == 0)
		{
			drn_check(Resource);
			Resource->Map(0, ReadRange, &ResourceBaseAddress);
		}
		else
		{
			drn_check(ResourceBaseAddress);
		}
		++NumMapCalls;

		return ResourceBaseAddress;
	}

	void RenderResource::Unmap()
	{
		drn_check(Resource);
		drn_check(ResourceBaseAddress);
		drn_check(NumMapCalls > 0);

		--NumMapCalls;
		if (NumMapCalls == 0)
		{
			Resource->Unmap(0, nullptr);
			ResourceBaseAddress = nullptr;
		}
	}

	void RenderResource::DeferDelete()
	{
		//GetParentDevice()->GetDeferredDeletionQueue().EnqueueResource(this, );
	}

	void RenderResource::ReleaseResource()
	{
		if (ShouldDeferDelete())
		{
			DeferDelete();
		}

		else
		{
			ReleaseResource();
		}
	}

}