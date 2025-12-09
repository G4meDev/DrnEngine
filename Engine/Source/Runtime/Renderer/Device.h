#pragma once

#include "ForwardTypes.h"
#include <dxgi1_6.h>

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

		void CreateCommittedResource(const D3D12_RESOURCE_DESC& InDesc, const D3D12_HEAP_PROPERTIES& HeapProps, D3D12_RESOURCE_STATES InInitialState, bool bNeedsStateTracking,
			const D3D12_CLEAR_VALUE* ClearValue, class RenderResource** ppOutResource, const std::string& Name);

		void CreateBuffer(D3D12_HEAP_TYPE HeapType, uint64 Size, D3D12_RESOURCE_STATES InInitialState, bool bNeedsStateTracking, class RenderResource** ppOutResource, const std::string& Name, D3D12_RESOURCE_FLAGS Flags);

	private:

		Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
		DXGI_ADAPTER_DESC3 m_Description;

		DeferredDeletionQueue m_DeferredDeletionQueue;
	};
}