#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/RefCounting.h"

namespace Drn
{
	class GpuFence : public RefCountedObject, public DeviceChild
	{
	public:
		GpuFence(Device* InParent, uint64 InitialValue, const std::string& InName = "<unnamed>");
		virtual ~GpuFence();

		inline ID3D12Fence* GetFence() const { return Fence.GetReference(); }
		inline HANDLE GetCompletionEvent() const { return hFenceCompleteEvent; }
		inline bool IsAvailable() const { return CompletedFence <= Fence->GetCompletedValue(); }

		uint64 Signal();

		//void GpuWait(uint32 DeviceGPUIndex, ED3D12CommandQueueType InQueueType, uint64 FenceValue, uint32 FenceGPUIndex);
		//void GpuWait(ED3D12CommandQueueType InQueueType, uint64 FenceValue);
		bool IsFenceComplete(uint64 FenceValue);
		void WaitForFence(uint64 FenceValue);

		bool IsFenceCompleteFast(uint64 FenceValue) const { return FenceValue <= CompletedFence; }
		virtual uint64 GetCurrentFence() const { return CurrentFence; }
		uint64 GetLastSignaledFence() const { return LastSignaledFence; }

		uint64 PeekCompletedFence() const { return Fence->GetCompletedValue(); };
		uint64 UpdateCompletedFence();


	protected:
		TRefCountPtr<ID3D12Fence> Fence; // TODO: try pooling
		HANDLE hFenceCompleteEvent;

		uint64 CurrentFence;
		uint64 LastSignaledFence;
		uint64 CompletedFence;

		std::string Name;

	};
}