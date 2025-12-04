#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/RefCounting.h"
#include "Runtime/Renderer/ResourceState.h"

namespace Drn
{
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

		inline void* Map(const D3D12_RANGE* ReadRange = nullptr);
		inline void Unmap();

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

		void SetName(const std::string& Name)
		{
			if (DebugName != Name)
			{
				DebugName = Name;
				::SetName(Resource, Name);
			}
		}

		std::string GetName() const
		{
			return DebugName;
		}

		void DoNotDeferDelete() { bDeferDelete = false; }
		inline bool ShouldDeferDelete() const { return bDeferDelete; }
		void DeferDelete();

		inline bool IsDepthStencilResource() const { return bDepthStencil; }

		void ReleaseResource();
	};
}