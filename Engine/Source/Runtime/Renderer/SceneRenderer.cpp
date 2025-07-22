#include "DrnPCH.h"
#include "SceneRenderer.h"

#include "Runtime/Engine/LightSceneProxy.h"
#include "Runtime/Renderer/RenderBuffer/HitProxyRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/TonemapRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/EditorPrimitiveRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/EditorSelectionRenderBuffer.h"

LOG_DEFINE_CATEGORY( LogSceneRenderer, "SceneRenderer" );

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	SceneRenderer::SceneRenderer(Scene* InScene)
		: m_Scene(InScene)
		, m_RenderingEnabled(true)
		, m_CachedRenderSize(1920, 1080)
		, m_CommandList(nullptr)
	{
		Init();
	}

	SceneRenderer::~SceneRenderer()
	{
		if (m_CommandList)
		{
			m_CommandList->ReleaseBufferedResource();
			m_CommandList = nullptr;
		}

#if WITH_EDITOR
		if (m_MousePickQueue.size() > 0)
		{
			LOG(LogSceneRenderer, Warning, "mouse pick event still has %i events", (int)m_MousePickQueue.size());
			__debugbreak();
		}

		OnSceneRendererDestroy.Braodcast();
#endif
	}

	void SceneRenderer::Init()
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		m_CommandList = new D3D12CommandList(Device, D3D12_COMMAND_LIST_TYPE_DIRECT, NUM_BACKBUFFERS, m_Name);
		m_CommandList->Close();

		m_GBuffer = std::make_shared<class GBuffer>();
		m_GBuffer->Init();

		m_TonemapBuffer = std::make_shared<class TonemapRenderBuffer>();
		m_TonemapBuffer->Init();

#if WITH_EDITOR
		// mouse picking components
		m_HitProxyRenderBuffer = std::make_shared<class HitProxyRenderBuffer>();
		m_HitProxyRenderBuffer->Init();
		Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_MousePickFence.GetAddressOf()));

		m_EditorPrimitiveBuffer = std::make_shared<class EditorPrimitiveRenderBuffer>();
		m_EditorPrimitiveBuffer->Init();

		m_EditorSelectionBuffer = std::make_shared<class EditorSelectionRenderBuffer>();
		m_EditorSelectionBuffer->Init();
#endif

		ResizeView(IntPoint(1920, 1080));
	}

	void SceneRenderer::BeginRender()
	{
		SCOPE_STAT();

		RecalculateView();
	}

#if WITH_EDITOR
	void SceneRenderer::RenderHitProxyPass()
	{
		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "HitProxy");

		m_HitProxyRenderBuffer->Clear( m_CommandList->GetD3D12CommandList() );
		m_HitProxyRenderBuffer->Bind(m_CommandList->GetD3D12CommandList());

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderHitProxyPass(m_CommandList->GetD3D12CommandList(), this);
		}

		PIXEndEvent(m_CommandList->GetD3D12CommandList());
	}
#endif

	void SceneRenderer::RenderBasePass()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "BasePass" );

		m_GBuffer->Clear( m_CommandList->GetD3D12CommandList() );
		m_GBuffer->Bind(m_CommandList->GetD3D12CommandList());

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(m_CommandList->GetD3D12CommandList(), this);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderLights()
	{
		SCOPE_STAT();

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_CommandList->GetD3D12CommandList()->SetPipelineState(CommonResources::Get()->m_LightPassPSO->m_PSO);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(CommonResources::Get()->m_LightPassPSO->m_RootSignature);

		m_GBuffer->BindLightPass(m_CommandList->GetD3D12CommandList());

		for ( LightSceneProxy* Proxy : m_Scene->m_LightProxies )
		{
#if WITH_EDITOR
			if (Proxy->m_SelectedInEditor)
			{
				Proxy->DrawAttenuation( m_Scene->GetWorld() );
			}
#endif

			Proxy->Render(m_CommandList->GetD3D12CommandList(), this);
		}

		m_GBuffer->UnBindLightPass(m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderPostProcess()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Post Process" );

		PostProcess_Tonemapping();

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_Tonemapping()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Tone mapping" );

		m_TonemapBuffer->Bind(m_CommandList->GetD3D12CommandList());
		
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);
		
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( CommonResources::Get()->m_TonemapPSO->m_RootSignature );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_TonemapPSO->m_PSO );
		
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_SceneView.LocalToCameraView, 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstants(0, 8, &m_SceneView.Size, 16);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootDescriptorTable(1, m_GBuffer->m_ColorDeferredSrvGpuHandle);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

#if WITH_EDITOR
	void SceneRenderer::RenderEditorPrimitives()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Editor Primitives" );
		m_EditorPrimitiveBuffer->Clear(m_CommandList->GetD3D12CommandList());
		m_EditorPrimitiveBuffer->Bind(m_CommandList->GetD3D12CommandList());

		ID3D12Resource* GBufferDepth = m_GBuffer->m_DepthTarget->GetD3D12Resource();
		ID3D12Resource* EditorPrimitiveDepth = m_EditorPrimitiveBuffer->m_DepthTarget->GetD3D12Resource();

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			GBufferDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		m_CommandList->GetD3D12CommandList()->CopyResource(EditorPrimitiveDepth, GBufferDepth);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			GBufferDepth, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveDepth, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);


		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderEditorPrimitivePass(m_CommandList->GetD3D12CommandList(), this);
		}

// ------------------------------------------------------------------------------------------

		ID3D12Resource* EditorPrimitiveColor = m_EditorPrimitiveBuffer->m_ColorTarget->GetD3D12Resource();

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &m_TonemapBuffer->m_TonemapHandle, true, NULL);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_RootSignature );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_SceneView.LocalToCameraView, 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootDescriptorTable(1, m_EditorPrimitiveBuffer->m_ColorSrvGpuHandle);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveColor, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderEditorSelection()
	{
		SCOPE_STAT();
		
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Editor Selection" );

		m_EditorSelectionBuffer->Clear(m_CommandList->GetD3D12CommandList());
		m_EditorSelectionBuffer->Bind(m_CommandList->GetD3D12CommandList());

		for ( PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies )
		{
			Proxy->RenderSelectionPass( m_CommandList->GetD3D12CommandList(), this);
		}

		ID3D12Resource* EditorSelectionDepth = m_EditorSelectionBuffer->m_DepthStencilTarget->GetD3D12Resource();

		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &m_TonemapBuffer->m_TonemapHandle, true, NULL );

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorSelectionDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( CommonResources::Get()->m_ResolveEditorSelectionPSO->m_RootSignature );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ResolveEditorSelectionPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstants(0, 16, &m_SceneView.LocalToCameraView, 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstants(0, 8, &m_SceneView.Size, 16);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootDescriptorTable(1, m_EditorSelectionBuffer->m_DepthStencilSrvGpuHandle);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorSelectionDepth, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		m_CommandList->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

#endif

	void SceneRenderer::Render()
	{
		SCOPE_STAT();

		m_CommandList->SetAllocatorAndReset(Renderer::Get()->m_SwapChain->GetBackBufferIndex());

		ID3D12DescriptorHeap* const Descs[2] = { Renderer::Get()->m_SrvHeap.Get(), Renderer::Get()->m_SamplerHeap.Get() };
		m_CommandList->GetD3D12CommandList()->SetDescriptorHeaps( 2, Descs );

		ResizeViewConditional();

#if WITH_EDITOR
		ProccessMousePickQueue();
#endif

		if (!m_RenderingEnabled)
		{
			m_CommandList->Close();
			return;
		}

#if WITH_EDITOR
		RenderHitProxyPass();
#endif

		BeginRender();
		RenderBasePass();
		RenderLights();
		RenderPostProcess();

#if WITH_EDITOR
		RenderEditorPrimitives();
		RenderEditorSelection();
#endif

		m_CommandList->Close();
	}

	ID3D12Resource* SceneRenderer::GetViewResource()
	{
		return m_TonemapBuffer->m_TonemapTarget->GetD3D12Resource();
	}

	void SceneRenderer::ResizeView( const IntPoint& InSize )
	{
		m_CachedRenderSize = IntPoint::ComponentWiseMax(InSize, IntPoint(1));
		m_RenderSize = m_CachedRenderSize;

		m_GBuffer->Resize( GetViewportSize() );
		m_TonemapBuffer->Resize( GetViewportSize() );

#if WITH_EDITOR
		m_HitProxyRenderBuffer->Resize( GetViewportSize() );
		m_EditorPrimitiveBuffer->Resize( GetViewportSize() );
		m_EditorSelectionBuffer->Resize( GetViewportSize() );
#endif

		OnSceneRendererResized.Braodcast(m_CachedRenderSize);
	}

	void SceneRenderer::ResizeViewDeferred( const IntPoint& InSize )
	{
		m_RenderSize = InSize;
	}

	void SceneRenderer::ResizeViewConditional()
	{
		if (m_RenderSize != m_CachedRenderSize)
		{
			ResizeView(m_RenderSize);
		}
	}

	void SceneRenderer::SetRenderingEnabled( bool Enabled )
	{
		m_RenderingEnabled = Enabled;
	}

	void SceneRenderer::RecalculateView()
	{
		float aspectRatio = (float) GetViewportSize().X / GetViewportSize().Y;
		m_CameraActor->GetCameraComponent()->CalculateMatrices(m_SceneView.WorldToView, m_SceneView.ViewToProjection, aspectRatio);
		
		m_SceneView.WorldToProjection = m_SceneView.WorldToView * m_SceneView.ViewToProjection;
		m_SceneView.ProjectionToView = XMMatrixInverse( NULL, m_SceneView.ViewToProjection.Get() );
		m_SceneView.ProjectionToWorld = XMMatrixInverse( NULL, m_SceneView.WorldToProjection.Get() );

		m_SceneView.LocalToCameraView = Matrix( m_CameraActor->GetActorTransform() ).Get() * m_SceneView.WorldToView.Get();
		
		m_SceneView.Size = GetViewportSize();
	}

#if WITH_EDITOR
	// TODO: support box selection
	void SceneRenderer::QueueMousePickEvent( const IntPoint& ScreenPosition )
	{
		m_MousePickQueue.emplace_back( ScreenPosition );
	}

	void SceneRenderer::ProccessMousePickQueue()
	{
		for ( auto it = m_MousePickQueue.begin(); it != m_MousePickQueue.end(); )
		{
			MousePickEvent& Event = *it;

			if (!Event.Initalized)
			{
				KickstartMousePickEvent(Event);
				it++;
			}

			else
			{
				if (Event.FenceValue >= m_MousePickFence->GetCompletedValue())
				{
					UINT8* MemoryStart;

					D3D12_RANGE ReadRange = {};
					ReadRange.Begin = 0;
					ReadRange.End = 16;
					Event.ReadbackBuffer->GetD3D12Resource()->Map(0, &ReadRange, reinterpret_cast<void**>(&MemoryStart));

					Guid Result;
					memcpy(&Result, MemoryStart, 16);

					World* W = GetScene() ? GetScene()->GetWorld() : nullptr;
					if (W)
					{
						if (OnPickedComponent.IsBound())
						{
							OnPickedComponent.Braodcast( W->GetComponentWithGuid(Result) );
						}
					}

					it = m_MousePickQueue.erase(it);
				}

				else
				{
					it++;
				}
			}
		}
	}

	void SceneRenderer::KickstartMousePickEvent( MousePickEvent& Event )
	{
		if ( m_HitProxyRenderBuffer && m_HitProxyRenderBuffer->m_GuidTarget)
		{
			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			Event.ReadbackBuffer = Resource::Create(D3D12_HEAP_TYPE_READBACK,
				CD3DX12_RESOURCE_DESC::Buffer( 16 ), D3D12_RESOURCE_STATE_COPY_DEST);

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_HitProxyRenderBuffer->m_GuidTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_COPY_SOURCE );
			m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );

			const IntPoint ClampedPos = IntPoint( std::clamp<int32>( Event.ScreenPos.X, 0, GetViewportSize().X - 1 ),
				std::clamp<int32>( Event.ScreenPos.Y, 0, GetViewportSize().Y - 1 ) );

			CD3DX12_BOX CopyBox( ClampedPos.X, ClampedPos.Y, ClampedPos.X + 1, ClampedPos.Y + 1 );

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
			Footprint.Footprint.Format                   = GBUFFER_GUID_FORMAT;
			Footprint.Footprint.Width                    = 1;
			Footprint.Footprint.Height                   = 1;
			Footprint.Footprint.Depth                    = 1;
			Footprint.Footprint.RowPitch                 = 256;
			Footprint.Offset                             = 0;

			CD3DX12_TEXTURE_COPY_LOCATION SourceLoc( m_HitProxyRenderBuffer->m_GuidTarget->GetD3D12Resource(), 0 );
			CD3DX12_TEXTURE_COPY_LOCATION DestLoc( Event.ReadbackBuffer->GetD3D12Resource(), Footprint );

			m_CommandList->GetD3D12CommandList()->CopyTextureRegion( &DestLoc, 0, 0, 0, &SourceLoc, &CopyBox );

			barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_HitProxyRenderBuffer->m_GuidTarget->GetD3D12Resource(),
															D3D12_RESOURCE_STATE_COPY_SOURCE,
															D3D12_RESOURCE_STATE_RENDER_TARGET );
			m_CommandList->GetD3D12CommandList()->ResourceBarrier( 1, &barrier );
			
			// push a fence
			Renderer::Get()->GetCommandQueue()->Signal( m_MousePickFence.Get(), ++m_FenceValue );
			Event.FenceValue = m_FenceValue;

			Event.Initalized = true;
		}

	}

#endif

}