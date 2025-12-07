#include "DrnPCH.h"
#include "RenderResource.h"

namespace Drn
{
#if D3D12_Debug_INFO
	std::atomic<int64> RenderResource::TotalResourceCount = 0;
	std::atomic<int64> RenderResource::NoStateTrackingResourceCount = 0;
#endif

	std::vector<SimpleRenderResource::ResourcesToDelete> SimpleRenderResource::DeferredDeletionQueue;
	uint32 SimpleRenderResource::CurrentFrame;

	void SimpleRenderResource::FlushPendingDeletes(bool bFlushDeferredDeletes)
	{
		auto Delete = [](std::vector<SimpleRenderResource*>& ToDelete)
		{
			for (int32 Index = 0; Index < ToDelete.size(); Index++)
			{
				SimpleRenderResource* Ref = ToDelete[Index];
				drn_check(Ref->bMarkedForDelete == 1);
				drn_check(Ref->GetRefCount() == 0);

				delete Ref;
			}
		};

		if (DeferredDeletionQueue.size())
		{
			if (bFlushDeferredDeletes)
			{
				Renderer::Get()->Flush();

				for (int32 Idx = 0; Idx < DeferredDeletionQueue.size(); ++Idx)
				{
					ResourcesToDelete& ResourceBatch = DeferredDeletionQueue[Idx];
					Delete(ResourceBatch.Resources);
				}

				DeferredDeletionQueue.clear();
			}
			else
			{
				int32 DeletedBatchCount = 0;
				while (DeletedBatchCount < DeferredDeletionQueue.size())
				{
					ResourcesToDelete& ResourceBatch = DeferredDeletionQueue[DeletedBatchCount];
					if (((ResourceBatch.FrameDeleted + NUM_BACKBUFFERS) < CurrentFrame))
					{
						Delete(ResourceBatch.Resources);
						++DeletedBatchCount;
					}
					else
					{
						break;
					}
				}

				if (DeletedBatchCount)
				{
					DeferredDeletionQueue.erase(DeferredDeletionQueue.begin(), DeferredDeletionQueue.begin() + DeletedBatchCount);
				}
			}

			++CurrentFrame;
		}
	}

	RenderResource::RenderResource( Device* ParentDevice, ID3D12Resource* InResource,D3D12_RESOURCE_DESC const& InDesc,
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
#if D3D12_Debug_INFO
		TotalResourceCount.fetch_add(-1);
#endif
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

	void RenderResource::SetName( const std::string& Name )
	{
		if ( DebugName != Name )
		{
			DebugName = Name;
			::SetName( Resource, Name );
		}
	}

	void RenderResource::DeferDelete()
	{
		GetParentDevice()->GetDeferredDeletionQueue().EnqueueResource(this, Renderer::Get()->GetFence());
	}

	void RenderResource::ReleaseResource()
	{
		if (ShouldDeferDelete())
		{
			DeferDelete();
		}

		else
		{
			this->Release();
		}
	}

// ---------------------------------------------------------------------------------------------------------------------

	ResourceLocation::ResourceLocation( Device* Parent )
		: DeviceChild(Parent)
		, Type(ResourceLocationType::eUndefined)
		, UnderlyingResource(nullptr)
		, MappedBaseAddress(nullptr)
		, GPUVirtualAddress(0)
		, Size(0)
		, bTransient(false)
	{}

	ResourceLocation::~ResourceLocation()
	{
		ReleaseResource();
	}

	void ResourceLocation::Clear()
	{
		InternalClear<true>();
	}

	void ResourceLocation::TransferOwnership( ResourceLocation& Destination, ResourceLocation& Source )
	{
		Destination.Clear();
		memmove(&Destination, &Source, sizeof(ResourceLocation));
		Source.InternalClear<false>();
	}

	void ResourceLocation::SetResource( RenderResource* Value )
	{
		drn_check(UnderlyingResource == nullptr);

		GPUVirtualAddress = Value->GetGPUVirtualAddress();
		UnderlyingResource = Value;
	}

	void ResourceLocation::AsStandAlone( RenderResource* Resource, uint32 BufferSize, bool bInIsTransient )
	{
		SetType(ResourceLocation::ResourceLocationType::eStandAlone);
		SetResource(Resource);
		SetSize(BufferSize);

		if (!IsCPUInaccessible(Resource->GetHeapType()))
		{
			D3D12_RANGE range = { 0, IsCPUWritable(Resource->GetHeapType())? 0 : BufferSize };
			SetMappedBaseAddress(Resource->Map(&range));
		}
		SetGPUVirtualAddress(Resource->GetGPUVirtualAddress());
		SetTransient(bInIsTransient);
	}

	template void ResourceLocation::InternalClear<false>();
	template void ResourceLocation::InternalClear<true>();

	template<bool bReleaseResource>
	void ResourceLocation::InternalClear()
	{
		if (bReleaseResource)
			ReleaseResource();

		Type = ResourceLocationType::eUndefined;
		UnderlyingResource = nullptr;
		MappedBaseAddress = nullptr;
		GPUVirtualAddress = 0;
		Size = 0;
	}

	void ResourceLocation::ReleaseResource()
	{
		switch ( Type )
		{
		case ResourceLocation::ResourceLocationType::eStandAlone:
		{
			drn_check(UnderlyingResource->GetRefCount() == 1);
			UnderlyingResource->ReleaseResource();

			break;
		}

		case ResourceLocation::ResourceLocationType::eUndefined:
		case ResourceLocation::ResourceLocationType::eSubAllocation:
		case ResourceLocation::ResourceLocationType::eFastAllocation:
		case ResourceLocation::ResourceLocationType::eMultiFrameFastAllocation:
		case ResourceLocation::ResourceLocationType::eAliased:
		case ResourceLocation::ResourceLocationType::eNodeReference:
		case ResourceLocation::ResourceLocationType::eHeapAliased:
		default:
			drn_check(false);
		}
	}

// ---------------------------------------------------------------------------------------------------------------------

	void BaseShaderResource::AddDynamicSRV( class BaseShaderResourceView* InSRV )
	{
		InSRV->DynamicResource = this;
		DynamicSRVs.push_back(InSRV);
	}

	void BaseShaderResource::RemoveDynamicSRV( class BaseShaderResourceView* InSRV )
	{
		drn_check(InSRV->DynamicResource == this);
		InSRV->DynamicResource = nullptr;
		std::erase(DynamicSRVs, InSRV);
	}

	void BaseShaderResource::RemoveAllDynamicSRVs()
	{
		for (BaseShaderResourceView* DynamicSRV : DynamicSRVs)
		{
			if (DynamicSRV != nullptr)
			{
				DynamicSRV->DynamicResource = nullptr;
			}
		}
		DynamicSRVs.clear();
	}

	BaseShaderResource::~BaseShaderResource()
	{
		for (BaseShaderResourceView* DynamicSRV : DynamicSRVs)
		{
			drn_check(DynamicSRV->DynamicResource == this);
			DynamicSRV->DynamicResource = nullptr;
		}
	}

}  // namespace Drn