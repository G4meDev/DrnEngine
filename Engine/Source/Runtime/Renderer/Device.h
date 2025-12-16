#pragma once

#include "ForwardTypes.h"
#include <dxgi1_6.h>
#include "Runtime/Renderer/RenderAllocation.h"

LOG_DECLARE_CATEGORY(LogDevice);

namespace Drn
{
	class DeferredDeletionQueue : public DeviceChild
	{
	public:
		using FencePair = std::pair<class GpuFence*, uint64>;

	private:
		enum class EObjectType
		{
			RHI,
			D3D,
		};

		struct FencedObjectType
		{
			union
			{
				class RenderResource* RHIObject;
				ID3D12Object*   D3DObject;
			};

			FencePair FencePair;
			EObjectType Type;
		};
		std::queue<FencedObjectType> DeferredReleaseQueue; // TODO: make thread safe

	public:

		void EnqueueResource(class RenderResource* pResource, class GpuFence* Fence);
		void EnqueueResource(ID3D12Object* pResource, class GpuFence* Fence);

		void ReleaseCompletedResources();
		void ReleaseResources();

		inline uint32 QueueSize() const { return DeferredReleaseQueue.size(); }

		DeferredDeletionQueue(class Device* InParent);
		~DeferredDeletionQueue();

	private:
	};

	class Device
	{
	public:

		Device();
		~Device();

		inline ID3D12Device* GetD3D12Device() const { return m_Device.Get(); }
		inline DeferredDeletionQueue& GetDeferredDeletionQueue() { return m_DeferredDeletionQueue; }

		inline DefaultBufferAllocator& GetDefaultBufferAllocator() { return DefaultBufferAllocator; }
		inline FastAllocator& GetDefaultFastAllocator() { return DefaultFastAllocator; }
		inline DynamicHeapAllocator& GetDynamicHeapAllocator() { return DynamicHeapAllocator; }
		inline FastConstantAllocator& GetTransientUniformBufferAllocator() { return FastConstantAllocator; }

		void CreateCommittedResource(const D3D12_RESOURCE_DESC& InDesc, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_RESOURCE_STATES InInitialState, bool bNeedsStateTracking,
			const D3D12_CLEAR_VALUE* ClearValue, class RenderResource** ppOutResource, const std::string& Name);

		void CreateBuffer(D3D12_HEAP_TYPE HeapType, uint64 Size, D3D12_RESOURCE_STATES InInitialState, bool bNeedsStateTracking, class RenderResource** ppOutResource, const std::string& Name, D3D12_RESOURCE_FLAGS Flags);

		void AllocateBuffer(const D3D12_RESOURCE_DESC& InDesc, uint32 Size, uint32 InUsage, bool bNeedsStateTracking,
			RenderResourceCreateInfo& CreateInfo, uint32 Alignment, ResourceLocation& Location);

		template <typename BufferType>
		BufferType* CreateRenderBuffer(class D3D12CommandList* CmdList, const D3D12_RESOURCE_DESC& Desc, uint32 Alignment, uint32 Stride, uint32 Size, uint32 InUsage,
			bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo);

		//class RenderIndexBuffer* CreateRenderIndexBuffer(class D3D12CommandList* CmdList, const D3D12_RESOURCE_DESC& Desc, uint32 Alignment, uint32 Stride, uint32 Size, uint32 InUsage,
		//	bool bNeedsStateTracking, RenderResourceCreateInfo& CreateInfo);

	private:

		Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
		DXGI_ADAPTER_DESC3 m_Description;

		DeferredDeletionQueue m_DeferredDeletionQueue;
		DefaultBufferAllocator DefaultBufferAllocator;
		FastAllocator DefaultFastAllocator;
		DynamicHeapAllocator DynamicHeapAllocator;
		FastConstantAllocator FastConstantAllocator;

		template <typename BufferType>
		static void UpdateBufferStats(ResourceLocation* Location, bool bAllocating);
	};

}