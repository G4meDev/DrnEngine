#include "DrnPCH.h"
#include "GpuFence.h"

namespace Drn
{
	GpuFence::GpuFence( Device* InParent, uint64 InitialValue, const std::string& InName )
		: DeviceChild(InParent)
		, hFenceCompleteEvent(INVALID_HANDLE_VALUE)
		, CurrentFence(1)
		, LastSignaledFence(0)
		, CompletedFence(0)
		, Name(InName)
	{
		hFenceCompleteEvent = CreateEvent(nullptr, false, false, nullptr);
		InParent->GetD3D12Device()->CreateFence(InitialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetInitReference()));

		::SetName(Fence, Name);
	}

	GpuFence::~GpuFence()
	{
		if (hFenceCompleteEvent != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFenceCompleteEvent);
			hFenceCompleteEvent = INVALID_HANDLE_VALUE;
		}

		if (Fence) { Fence->Release(); }
	}

	uint64 GpuFence::Signal()
	{
		drn_check(LastSignaledFence != CurrentFence);

		Renderer::Get()->GetCommandQueue()->Signal(Fence, CurrentFence);
		LastSignaledFence = CurrentFence;
		UpdateCompletedFence();
		CurrentFence++;

		return LastSignaledFence;
	}

	uint64 GpuFence::UpdateCompletedFence()
	{
		CompletedFence = PeekCompletedFence();
		return CompletedFence;
	}

	bool GpuFence::IsFenceComplete( uint64 FenceValue )
	{
		drn_check(FenceValue <= CurrentFence);
		if (FenceValue <= CompletedFence)
		{
			return true;
		}

		return FenceValue <= UpdateCompletedFence();
	}

	void GpuFence::WaitForFence( uint64 FenceValue )
	{
		if (!IsFenceComplete(FenceValue) && FenceValue > PeekCompletedFence())
		{
			SCOPE_STAT("FenceWait");

			Fence->SetEventOnCompletion(FenceValue, hFenceCompleteEvent);
			const uint32 WaitResult = WaitForSingleObject(hFenceCompleteEvent, INFINITE);
			drn_check(WaitResult == 0);

			UpdateCompletedFence();
		}
	}

}