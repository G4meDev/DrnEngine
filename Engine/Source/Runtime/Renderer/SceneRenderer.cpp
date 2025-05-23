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
		, m_RenderSize(1920, 1080)
	{
		Init(Renderer::Get()->GetCommandList());
	}

	SceneRenderer::~SceneRenderer()
	{
#if WITH_EDITOR
		if (m_MousePickQueue.size() > 0)
		{
			LOG(LogSceneRenderer, Warning, "mouse pick event still has %i events", (int)m_MousePickQueue.size());
			__debugbreak();
		}
#endif
	}

	void SceneRenderer::Init( ID3D12GraphicsCommandList2* CommandList )
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

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

	void SceneRenderer::BeginRender(ID3D12GraphicsCommandList2* CommandList)
	{
		SCOPE_STAT( BeginRender );

	}

#if WITH_EDITOR
	void SceneRenderer::RenderHitProxyPass( ID3D12GraphicsCommandList2* CommandList )
	{
		PIXBeginEvent(CommandList, 1, "HitProxy");

		m_HitProxyRenderBuffer->Clear(CommandList);
		m_HitProxyRenderBuffer->Bind(CommandList);

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderHitProxyPass(CommandList, this);
		}

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_EditorPrimitiveProxies)
		{
			Proxy->RenderHitProxyPass(CommandList, this);
		}

		PIXEndEvent(CommandList);
	}
#endif

	void SceneRenderer::RenderBasePass( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT( RenderBasePass );

		PIXBeginEvent( CommandList, 1, "BasePass" );

		m_GBuffer->Clear(CommandList);
		m_GBuffer->Bind(CommandList);

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(CommandList, this);
		}

		PIXEndEvent( CommandList );
	}

	void SceneRenderer::RenderLights( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT( RenderLights );

		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandList->SetPipelineState(CommonResources::Get()->m_LightPassPSO->m_PSO);
		CommandList->SetGraphicsRootSignature(CommonResources::Get()->m_LightPassPSO->m_RootSignature);

		m_GBuffer->BindLightPass(CommandList);

		for ( LightSceneProxy* Proxy : m_Scene->m_LightProxies )
		{
			if (Proxy->m_SelectedInEditor)
			{
				Proxy->DrawAttenuation( m_Scene->GetWorld() );
			}

			Proxy->Render(CommandList, this);
		}

		m_GBuffer->UnBindLightPass(CommandList);
	}

	void SceneRenderer::RenderPostProcess( ID3D12GraphicsCommandList2* CommandList )
	{
		PIXBeginEvent( CommandList, 1, "Post Process" );

		PostProcess_Tonemapping(CommandList);

		PIXEndEvent( CommandList );
	}

	void SceneRenderer::PostProcess_Tonemapping( ID3D12GraphicsCommandList2* CommandList )
	{
		PIXBeginEvent( CommandList, 1, "Tone mapping" );

		m_TonemapBuffer->Bind(CommandList);
		
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		CommandList->ResourceBarrier(1, &barrier);
		
		CommandList->SetGraphicsRootSignature( CommonResources::Get()->m_TonemapPSO->m_RootSignature );
		CommandList->SetPipelineState( CommonResources::Get()->m_TonemapPSO->m_PSO );
		
		XMMATRIX modelMatrix = Matrix( m_CameraActor->GetActorTransform() ).Get();
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		float aspectRatio = (float) m_RenderSize.X / m_RenderSize.Y;
		m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		XMMATRIX LocalToView = XMMatrixMultiply( modelMatrix, viewMatrix );
		
		uint32 RenderSize[2] = { (uint32)m_RenderSize.X, (uint32)m_RenderSize.Y};
		CommandList->SetGraphicsRoot32BitConstants(0, 16, &LocalToView, 0);
		CommandList->SetGraphicsRoot32BitConstants(0, 8, &RenderSize[0], 16);
		CommandList->SetGraphicsRootDescriptorTable(1, m_GBuffer->m_ColorDeferredSrvGpuHandle);

		CommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommandList->IASetVertexBuffers( 0, 1, &CommonResources::Get()->m_ScreenTriangle->m_VertexBufferView );
		CommandList->IASetIndexBuffer( &CommonResources::Get()->m_ScreenTriangle->m_IndexBufferView );
		CommandList->DrawIndexedInstanced( 3, 1, 0, 0, 0 );
		
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		CommandList->ResourceBarrier(1, &barrier);

		PIXEndEvent( CommandList );
	}

#if WITH_EDITOR
	void SceneRenderer::RenderEditorPrimitives( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT( RenderEditorPrimitives );

		PIXBeginEvent( CommandList, 1, "Editor Primitives" );
		m_EditorPrimitiveBuffer->Clear(CommandList);
		m_EditorPrimitiveBuffer->Bind(CommandList);

		ID3D12Resource* GBufferDepth = m_GBuffer->m_DepthTarget->GetD3D12Resource();
		ID3D12Resource* EditorPrimitiveDepth = m_EditorPrimitiveBuffer->m_DepthTarget->GetD3D12Resource();

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			GBufferDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE );
		CommandList->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST );
		CommandList->ResourceBarrier(1, &barrier);

		CommandList->CopyResource(EditorPrimitiveDepth, GBufferDepth);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			GBufferDepth, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		CommandList->ResourceBarrier(1, &barrier);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveDepth, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		CommandList->ResourceBarrier(1, &barrier);


		for (PrimitiveSceneProxy* Proxy : m_Scene->m_EditorPrimitiveProxies)
		{
			Proxy->RenderEditorPrimitivePass(CommandList, this);
		}

// ------------------------------------------------------------------------------------------

		ID3D12Resource* EditorPrimitiveColor = m_EditorPrimitiveBuffer->m_ColorTarget->GetD3D12Resource();

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		CommandList->ResourceBarrier(1, &barrier);

		CommandList->OMSetRenderTargets(1, &m_TonemapBuffer->m_TonemapRtvHeap->GetCPUDescriptorHandleForHeapStart(), true, NULL);
		CommandList->SetGraphicsRootSignature( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_RootSignature );
		CommandList->SetPipelineState( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_PSO );

		XMMATRIX modelMatrix = Matrix( m_CameraActor->GetActorTransform() ).Get();
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		float aspectRatio = (float) m_RenderSize.X / m_RenderSize.Y;
		m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		XMMATRIX LocalToView = XMMatrixMultiply( modelMatrix, viewMatrix );

		CommandList->SetGraphicsRoot32BitConstants(0, 16, &LocalToView, 0);
		CommandList->SetGraphicsRootDescriptorTable(1, m_EditorPrimitiveBuffer->m_ColorSrvGpuHandle);

		CommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommandList->IASetVertexBuffers( 0, 1, &CommonResources::Get()->m_ScreenTriangle->m_VertexBufferView );
		CommandList->IASetIndexBuffer( &CommonResources::Get()->m_ScreenTriangle->m_IndexBufferView );
		CommandList->DrawIndexedInstanced( 3, 1, 0, 0, 0 );

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorPrimitiveColor, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET );
		CommandList->ResourceBarrier(1, &barrier);

		PIXEndEvent( CommandList );
	}

	void SceneRenderer::RenderEditorSelection( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT(RenderEditorSelection);
		
		PIXBeginEvent( CommandList, 1, "Editor Selection" );

		XMMATRIX modelMatrix = Matrix( m_CameraActor->GetActorTransform() ).Get();
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		float aspectRatio = (float) m_RenderSize.X / m_RenderSize.Y;
		m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		XMMATRIX LocalToView = XMMatrixMultiply( modelMatrix, viewMatrix );

		m_EditorSelectionBuffer->Clear(CommandList);
		m_EditorSelectionBuffer->Bind(CommandList);

		for ( PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies )
		{
			Proxy->RenderSelectionPass( CommandList, this);
		}

		ID3D12Resource* EditorSelectionDepth = m_EditorSelectionBuffer->m_DepthStencilTarget->GetD3D12Resource();

		CommandList->OMSetRenderTargets( 1, &m_TonemapBuffer->m_TonemapRtvHeap->GetCPUDescriptorHandleForHeapStart(), true, NULL );

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorSelectionDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		CommandList->ResourceBarrier(1, &barrier);

		CommandList->SetGraphicsRootSignature( CommonResources::Get()->m_ResolveEditorSelectionPSO->m_RootSignature );
		CommandList->SetPipelineState( CommonResources::Get()->m_ResolveEditorSelectionPSO->m_PSO );

		uint32 RenderSize[2] = { (uint32)m_RenderSize.X, (uint32)m_RenderSize.Y};
		CommandList->SetGraphicsRoot32BitConstants(0, 16, &LocalToView, 0);
		CommandList->SetGraphicsRoot32BitConstants(0, 8, &RenderSize[0], 16);
		CommandList->SetGraphicsRootDescriptorTable(1, m_EditorSelectionBuffer->m_DepthStencilSrvGpuHandle);

		CommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommandList->IASetVertexBuffers( 0, 1, &CommonResources::Get()->m_ScreenTriangle->m_VertexBufferView );
		CommandList->IASetIndexBuffer( &CommonResources::Get()->m_ScreenTriangle->m_IndexBufferView );
		CommandList->DrawIndexedInstanced( 3, 1, 0, 0, 0 );

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			EditorSelectionDepth, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		CommandList->ResourceBarrier(1, &barrier);

		PIXEndEvent( CommandList );
	}

#endif

	void SceneRenderer::Render( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT(SceneRendererRender);

#if WITH_EDITOR
		ProccessMousePickQueue( CommandList );
#endif

		if (!m_RenderingEnabled)
		{
			return;
		}

#if WITH_EDITOR
		RenderHitProxyPass(CommandList);
#endif

		BeginRender(CommandList);
		RenderBasePass(CommandList);
		RenderLights(CommandList);
		RenderPostProcess(CommandList);

#if WITH_EDITOR
		RenderEditorPrimitives(CommandList);
		RenderEditorSelection(CommandList);
#endif
	}

	ID3D12Resource* SceneRenderer::GetViewResource()
	{
		return m_TonemapBuffer->m_TonemapTarget->GetD3D12Resource();
	}

	void SceneRenderer::ResizeView( const IntPoint& InSize )
	{
		Renderer::Get()->Flush();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		m_RenderSize = IntPoint::ComponentWiseMax(InSize, IntPoint(1));

		m_GBuffer->Resize( m_RenderSize );
		m_TonemapBuffer->Resize( m_RenderSize );

#if WITH_EDITOR
		m_HitProxyRenderBuffer->Resize( m_RenderSize );
		m_EditorPrimitiveBuffer->Resize( m_RenderSize );
		m_EditorSelectionBuffer->Resize( m_RenderSize );
#endif
	}

	void SceneRenderer::SetRenderingEnabled( bool Enabled )
	{
		m_RenderingEnabled = Enabled;
	}


#if WITH_EDITOR
	// TODO: support box selection
	void SceneRenderer::QueueMousePickEvent( const IntPoint& ScreenPosition )
	{
		m_MousePickQueue.emplace_back( ScreenPosition );
	}

	void SceneRenderer::ProccessMousePickQueue( ID3D12GraphicsCommandList2* CommandList )
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
			ID3D12GraphicsCommandList2* CommandList = Renderer::Get()->GetCommandList();

			Event.ReadbackBuffer = Resource::Create(D3D12_HEAP_TYPE_READBACK,
				CD3DX12_RESOURCE_DESC::Buffer( 16 ), D3D12_RESOURCE_STATE_COPY_DEST);

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_HitProxyRenderBuffer->m_GuidTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_COPY_SOURCE );
			CommandList->ResourceBarrier( 1, &barrier );

			const IntPoint ClampedPos = IntPoint( std::clamp<int32>( Event.ScreenPos.X, 0, m_RenderSize.X - 1 ),
				std::clamp<int32>( Event.ScreenPos.Y, 0, m_RenderSize.Y - 1 ) );

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

			CommandList->CopyTextureRegion( &DestLoc, 0, 0, 0, &SourceLoc, &CopyBox );

			barrier = CD3DX12_RESOURCE_BARRIER::Transition( m_HitProxyRenderBuffer->m_GuidTarget->GetD3D12Resource(),
															D3D12_RESOURCE_STATE_COPY_SOURCE,
															D3D12_RESOURCE_STATE_RENDER_TARGET );
			CommandList->ResourceBarrier( 1, &barrier );
			
			// push a fence
			Renderer::Get()->GetCommandQueue()->Signal( m_MousePickFence.Get(), ++m_FenceValue );
			Event.FenceValue = m_FenceValue;

			Event.Initalized = true;
		}

	}

#endif

}