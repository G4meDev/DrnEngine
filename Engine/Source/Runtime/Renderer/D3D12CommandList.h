#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderResource.h"
#include "Runtime/Renderer/RenderCommon.h"
#include "Runtime/Renderer/RenderTexture.h"
#include "Runtime/Renderer/RenderStateCache.h"

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
		std::unordered_map<class RenderResource*, ResourceState> ResourceStates;
		void inline ConditionalInitalize(class RenderResource* pResource, ResourceState& ResourceState);

	public:
		ResourceState& GetResourceState(class RenderResource* pResource);
		void Empty();
	};

	class D3D12CommandList : public SimpleRenderResource, public DeviceChild
	{
	public:
		D3D12CommandList(class Device* Device, D3D12_COMMAND_LIST_TYPE Type, uint8 NumAllocators, const std::string& Name);
		virtual ~D3D12CommandList();

		virtual uint32 AddRef() const { return SimpleRenderResource::AddRef(); }
		virtual uint32 Release() const { return SimpleRenderResource::Release(); }
		virtual uint32 GetRefCount() const { return SimpleRenderResource::GetRefCount(); }

		inline ID3D12GraphicsCommandList2* GetD3D12CommandList() { return m_CommandList.Get(); }

		void Close();
		void FlipAndReset();
		void SetAllocatorAndReset(uint8 AllocatorIndex);

		void ClearDepthTexture( class DepthStencilView* InView, bool bClearDepth, float Depth, bool bClearStencil, uint8 Stencil );
		void ClearDepthTexture( class RenderTextureBase* InTexture, EDepthStencilViewType Type, bool bClearDepth, float Depth, bool bClearStencil, uint8 Stencil );
		void ClearDepthTexture( class RenderTextureBase* InTexture, EDepthStencilViewType Type, bool bClearDepth, bool bClearStencil );

		void ClearColorTexture( class RenderTextureBase* InTexture, int32 MipIndex, int32 SliceIndex, Vector4 ClearValue );
		void ClearColorTexture( class RenderTextureBase* InTexture, int32 MipIndex = 0, int32 SliceIndex = 0 );

		void AddTransitionBarrier(class RenderResource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource);
		void FlushBarriers();

		void TransitionResourceWithTracking(class RenderResource* pResource, D3D12_RESOURCE_STATES After, uint32 Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		//void TransitionResourceWithTracking(class RenderResource* pResource, D3D12_RESOURCE_STATES After, const CViewSubresourceSubset& SubresourceSubset);
		ResourceState& GetResourceState(class RenderResource* pResource);

		void SetIndexBuffer(const ResourceLocation& IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset);
		void SetStreamSource(uint32 StreamIndex, class RenderVertexBuffer* VertexBuffer, uint32 Offset);

		// TODO: remove. this should happen when a new pipeline state is set
		void SetStreamStrides(const uint16* Strides);
		void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY Topology);

		void SetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ);
		void SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY);

		void DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances);
		void DrawIndexedPrimitive(class RenderIndexBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances);

		void ClearState();

	protected:

	private:

		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_CommandList;

		D3D12_COMMAND_LIST_TYPE m_Type;
		uint8 m_NumAllocators;

		uint8 m_CurrentAllocatorIndex;

		ResourceBarrierBatcher m_ResourceBarrierBatcher;
		CommandListResourceState TrackedResourceState;

		RenderStateCache StateCache;
	};

	class ConditionalScopeResourceBarrier
	{
	private:
		D3D12CommandList* CmdList;
		class RenderResource* const pResource;
		D3D12_RESOURCE_STATES Current;
		const D3D12_RESOURCE_STATES Desired;
		const uint32 Subresource;
		bool bRestoreDefaultState;

	public:
		explicit ConditionalScopeResourceBarrier(D3D12CommandList* InCmdList, class RenderResource* pInResource, const D3D12_RESOURCE_STATES InDesired, const uint32 InSubresource);
		~ConditionalScopeResourceBarrier();
	};

// -------------------------------------------------------------------------------------------------

	class PrimitiveStats
	{
	public:

		enum class EPrimitiveStatGroups : uint8
		{
			Triangle,
			Line,
			Max
		};

		static void UpdateStats(uint64 Count, D3D_PRIMITIVE_TOPOLOGY Topology);
		static void UpdateStats(uint64 Count, EPrimitiveStatGroups Group);
		static void ClearStats();

		static std::string GetStatName(EPrimitiveStatGroups Group);
		static int32 GetStatSize(EPrimitiveStatGroups Group);

	private:
#if RENDER_STATS
		static std::atomic<uint64> Stats[(uint8)EPrimitiveStatGroups::Max];
#endif
	};
}