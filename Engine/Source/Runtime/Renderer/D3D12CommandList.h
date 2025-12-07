#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/BufferedResource.h"
#include "Runtime/Renderer/RenderCommon.h"

namespace Drn
{
	static bool IsTransitionNeeded(D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES& After);

	class ResourceBarrierBatcher : public Noncopyable
	{
	public:
		explicit ResourceBarrierBatcher()
		{};

		//void AddUAV()
		//{
		//	Barriers.AddUninitialized();
		//	D3D12_RESOURCE_BARRIER& Barrier = Barriers.Last();
		//	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		//	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//	Barrier.UAV.pResource = nullptr;	// Ignore the resource ptr for now. HW doesn't do anything with it.
		//}

		// Add a transition resource barrier to the batch. Returns the number of barriers added, which may be negative if an existing barrier was cancelled.
		int32 AddTransition(class RenderResource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource);

		//void AddAliasingBarrier(ID3D12Resource* pResource)
		//{
		//	Barriers.AddUninitialized();
		//	D3D12_RESOURCE_BARRIER& Barrier = Barriers.Last();
		//	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		//	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//	Barrier.Aliasing.pResourceBefore = NULL;
		//	Barrier.Aliasing.pResourceAfter = pResource;
		//}

		void Flush(class ID3D12GraphicsCommandList* pCommandList, int32 BarrierBatchMax = INT32_MAX);
		void Reset() { Barriers.clear(); }

		const std::vector<D3D12_RESOURCE_BARRIER>& GetBarriers() const
		{
			return Barriers;
		}

	private:
		std::vector<D3D12_RESOURCE_BARRIER> Barriers;
	};

	class CommandListResourceState
	{
	private:
		std::unordered_map<class RenderResource*, ResourceState_New> ResourceStates;
		void inline ConditionalInitalize(class RenderResource* pResource, ResourceState_New& ResourceState);

	public:
		ResourceState_New& GetResourceState(class RenderResource* pResource);
		void Empty();
	};


	class D3D12CommandList : public BufferedResource, public DeviceChild
	{
	public:
		D3D12CommandList(class Device* Device, D3D12_COMMAND_LIST_TYPE Type, uint8 NumAllocators, const std::string& Name);
		virtual ~D3D12CommandList();

		inline ID3D12GraphicsCommandList2* GetD3D12CommandList() { return m_CommandList.Get(); }

		void Close();
		void FlipAndReset();
		void SetAllocatorAndReset(uint8 AllocatorIndex);

		void ClearColorTexture( class RenderTextureBase* InTexture, int32 MipIndex, int32 SliceIndex, Vector4 ClearValue );
		void ClearColorTexture( class RenderTextureBase* InTexture, int32 MipIndex = 0, int32 SliceIndex = 0 );

		void AddTransitionBarrier(class RenderResource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource);
		void FlushBarriers();

		void TransitionResourceWithTracking(class RenderResource* pResource, D3D12_RESOURCE_STATES After, uint32 Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		//void TransitionResourceWithTracking(class RenderResource* pResource, D3D12_RESOURCE_STATES After, const CViewSubresourceSubset& SubresourceSubset);
		ResourceState_New& GetResourceState(class RenderResource* pResource);

	protected:

	private:

		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_CommandList;

		D3D12_COMMAND_LIST_TYPE m_Type;
		uint8 m_NumAllocators;

		uint8 m_CurrentAllocatorIndex;

		ResourceBarrierBatcher m_ResourceBarrierBatcher;
		CommandListResourceState TrackedResourceState;
	};
}