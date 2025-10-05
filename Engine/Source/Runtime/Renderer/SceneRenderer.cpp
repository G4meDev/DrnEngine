#include "DrnPCH.h"
#include "SceneRenderer.h"

#include "Runtime/Engine/LightSceneProxy.h"
#include "Runtime/Renderer/RenderBuffer/HitProxyRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/HZBBuffer.h"
#include "Runtime/Renderer/RenderBuffer/TonemapRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/EditorPrimitiveRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/EditorSelectionRenderBuffer.h"
#include "Runtime/Renderer/RenderBuffer/RenderBufferAO.h"
#include "Runtime/Renderer/RenderBuffer/ScreenSpaceReflectionBuffer.h"
#include "Runtime/Renderer/RenderBuffer/ReflectionEnvironmentBuffer.h"
#include "Runtime/Renderer/RenderBuffer/TAABuffer.h"

#include "Runtime/Engine/PostProcessVolume.h"

LOG_DEFINE_CATEGORY( LogSceneRenderer, "SceneRenderer" );

#define HZB_GROUP_TILE_SIZE 8

using namespace DirectX;
using namespace Microsoft::WRL;

namespace Drn
{
	SceneRenderer::SceneRenderer(Scene* InScene)
		: m_Scene(InScene)
		, m_RenderingEnabled(true)
		, m_CachedRenderSize(1920, 1080)
		, m_CommandList(nullptr)
		, m_FrameIndex(0)
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

		for (int32 i = 0; i < NUM_BACKBUFFERS; i++)
		{
			if (m_BindlessViewBuffer[i])
			{
				m_BindlessViewBuffer[i]->ReleaseBufferedResource();
				m_BindlessViewBuffer[i] = nullptr;
			}
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

		m_HZBBuffer = std::make_shared<class HZBBuffer>();
		m_HZBBuffer->Init();

		m_TonemapBuffer = std::make_shared<class TonemapRenderBuffer>();
		m_TonemapBuffer->Init();

		m_AOBuffer = std::make_shared<class RenderBufferAO>();
		m_AOBuffer->Init();

		m_ScreenSpaceReflectionBuffer = std::make_shared<class ScreenSpaceReflectionBuffer>();
		m_ScreenSpaceReflectionBuffer->Init();

		m_ReflectionEnvironmentBuffer = std::make_shared<class ReflectionEnvironmentBuffer>();
		m_ReflectionEnvironmentBuffer->Init();

		m_TAABuffer = std::make_shared<class TAABuffer>();
		m_TAABuffer->Init();

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

		m_RenderTask.emplace( [&]() { Render(); } );

		for (int32 i = 0; i < NUM_BACKBUFFERS; i++)
		{
			m_BindlessViewBuffer[i] = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 1024 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
			m_BindlessViewBuffer[i]->SetName("SceneViewBuffer_" + m_Name);
#endif

			D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.BufferLocation = m_BindlessViewBuffer[i]->GetD3D12Resource()->GetGPUVirtualAddress();
			ResourceViewDesc.SizeInBytes = 1024;
			Device->CreateConstantBufferView( &ResourceViewDesc, m_BindlessViewBuffer[i]->GetCpuHandle());
		}

	}

	void SceneRenderer::ResolvePostProcessSettings()
	{
		SCOPE_STAT();

		m_PostProcessSettings = &PostProcessSettings::DefaultSettings;

		float MinDistance = FLT_MAX;
		for (auto it = m_Scene->m_PostProcessProxies.begin(); it != m_Scene->m_PostProcessProxies.end(); it++)
		{
			PostProcessSceneProxy* Proxy = *it;
			float Dist  = Vector::Distance(Proxy->m_WorldTransform.GetLocation(), m_SceneView.CameraPos);

			if (Dist < MinDistance)
			{
				m_PostProcessSettings = &Proxy->m_Settings;
				MinDistance = Dist;
			}
		}
	}

	void SceneRenderer::UpdateViewBuffer()
	{
		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_SceneView, sizeof(SceneRendererView));
		m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetD3D12Resource()->Unmap(0, nullptr);
	}

	void SceneRenderer::RenderShadowDepths()
	{
		SCOPE_STAT();

		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "ShadowDepths");

		for ( LightSceneProxy* Proxy : m_Scene->m_LightProxies )
		{
			Proxy->RenderShadowDepth(m_CommandList->GetD3D12CommandList(), this);
		}

		PIXEndEvent(m_CommandList->GetD3D12CommandList());
	}

#if WITH_EDITOR
	void SceneRenderer::RenderHitProxyPass()
	{
		SCOPE_STAT();

		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "HitProxy");

		ResourceStateTracker::Get()->TransiationResource(m_HitProxyRenderBuffer->m_DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		ResourceStateTracker::Get()->TransiationResource(m_HitProxyRenderBuffer->m_GuidTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());

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

		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_DepthTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_ColorDeferredTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_BaseColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_WorldNormalTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_MasksTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_VelocityTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());

		m_GBuffer->Clear( m_CommandList->GetD3D12CommandList() );
		m_GBuffer->Bind(m_CommandList->GetD3D12CommandList());

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(m_CommandList->GetD3D12CommandList(), this);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderHZB()
	{
		SCOPE_STAT();
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "HZB" );

		ResourceStateTracker::Get()->TransiationResource(m_GBuffer->m_DepthTarget, D3D12_RESOURCE_STATE_DEPTH_READ);
		ResourceStateTracker::Get()->TransiationResource(m_HZBBuffer->M_HZBTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		int32 RemainingMips = m_HZBBuffer->m_MipCount;

		const int32 OutputIndexStart = 3;
		while (RemainingMips > 0)
		{
			int32 DispatchStartMipIndex = m_HZBBuffer->m_MipCount - RemainingMips;
			int32 DispatchMipCount = RemainingMips > 4 ? 4 : RemainingMips;
			RemainingMips -= DispatchMipCount;

			const IntPoint MipSize = m_HZBBuffer->m_FirstMipSize / std::pow<int32>(2, DispatchStartMipIndex);
			IntPoint DispatchSize = MipSize / HZB_GROUP_TILE_SIZE;
			DispatchSize = IntPoint::ComponentWiseMax(DispatchSize, IntPoint(1));

			D3D12_GPU_DESCRIPTOR_HANDLE ParentViewHandle = DispatchStartMipIndex == 0 ?
				m_GBuffer->m_DepthTarget->GetGpuHandle() :
				m_HZBBuffer->m_SrvHandles[DispatchStartMipIndex / 4 - 1].GpuHandle;

			if ( DispatchStartMipIndex != 0)
			{
				ResourceStateTracker::Get()->TransiationResource(m_HZBBuffer->M_HZBTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, DispatchStartMipIndex - 1);
			}

			ID3D12PipelineState* DispatchPSO = nullptr;

			if		( DispatchMipCount == 4)	{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_4Mip_PSO; }
			else if ( DispatchMipCount == 3)	{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_3Mip_PSO; }
			else if ( DispatchMipCount == 2)	{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_2Mip_PSO; }
			else 								{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_1Mip_PSO; }

			m_CommandList->GetD3D12CommandList()->SetComputeRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			m_CommandList->GetD3D12CommandList()->SetPipelineState(DispatchPSO);

			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(ParentViewHandle), 1);
			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

			Vector4 DispatchIDToUV;
			Vector InvSize = Vector(1.0f / MipSize.X, 1.0f / MipSize.Y, 0);

			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstants(0, 4, &DispatchIDToUV, 8);
			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstants(0, 3, &InvSize, 12);

			for (int32 i = 0; i < DispatchMipCount; i++)
			{
				const int32 MipIndex = DispatchStartMipIndex + i;
				m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_HZBBuffer->m_UAVHandles[MipIndex].GpuHandle), OutputIndexStart + i);
			}

			ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
			m_CommandList->GetD3D12CommandList()->Dispatch(DispatchSize.X, DispatchSize.Y, 1);
 		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderAO()
	{
		SCOPE_STAT();
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "AO" );

		m_AOBuffer->MapBuffer(m_CommandList->GetD3D12CommandList(), this, m_PostProcessSettings->m_SSAOSettings);

		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_DEPTH_READ);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_WorldNormalTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_HZBBuffer->M_HZBTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOSetupTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOHalfTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

		{
			m_AOBuffer->BindSetup(m_CommandList->GetD3D12CommandList());

			m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_AmbientOcclusionPSO->m_SetupPSO );

			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_AOBuffer->m_AoBuffer->GetGpuHandle()), 1);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

			ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
			m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());
		}

		{
			ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOSetupTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			m_AOBuffer->BindHalf(m_CommandList->GetD3D12CommandList());

			m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_AmbientOcclusionPSO->m_HalfPSO );


			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_AOBuffer->m_AoBuffer->GetGpuHandle()), 1);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

			ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
			m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());
		}

		{
			ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOHalfTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			m_AOBuffer->BindMain(m_CommandList->GetD3D12CommandList());

			m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_AmbientOcclusionPSO->m_MainPSO );

			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_AOBuffer->m_AoBuffer->GetGpuHandle()), 1);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

			ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
			m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderLights()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "LightPass" );

		ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		m_CommandList->GetD3D12CommandList()->SetPipelineState(CommonResources::Get()->m_LightPassPSO->m_PSO);

		m_GBuffer->BindLightPass(m_CommandList->GetD3D12CommandList());

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		// 1 is light buffer
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_GBuffer->m_BaseColorTarget->GetGpuHandle()), 3);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_GBuffer->m_WorldNormalTarget->GetGpuHandle()), 4);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_GBuffer->m_MasksTarget->GetGpuHandle()), 5);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_GBuffer->m_DepthTarget->GetGpuHandle()), 6);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_AOBuffer->m_AOTarget->GetGpuHandle()), 8);

		for ( LightSceneProxy* Proxy : m_Scene->m_LightProxies )
		{
			Proxy->Render(m_CommandList->GetD3D12CommandList(), this);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderSSR()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Screen Space Reflection" );

		m_ScreenSpaceReflectionBuffer->MapBuffer(m_CommandList->GetD3D12CommandList(), this, m_PostProcessSettings->m_SSRSettings);

		ResourceStateTracker::Get()->TransiationResource( m_ScreenSpaceReflectionBuffer->m_Target, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_BaseColorTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_WorldNormalTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_MasksTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_DEPTH_READ);
		ResourceStateTracker::Get()->TransiationResource( m_HZBBuffer->M_HZBTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		m_ScreenSpaceReflectionBuffer->Bind(m_CommandList->GetD3D12CommandList());
		m_ScreenSpaceReflectionBuffer->Clear(m_CommandList->GetD3D12CommandList());

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ScreenSpaceReflectionPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ScreenSpaceReflectionBuffer->m_Buffer->GetGpuHandle()), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderReflection()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Reflection Environment" );

		m_ReflectionEnvironmentBuffer->MapBuffer(m_CommandList->GetD3D12CommandList(), this);
		
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->TransiationResource( m_ScreenSpaceReflectionBuffer->m_Target, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_AOBuffer->m_AOTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_BaseColorTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_WorldNormalTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_MasksTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_DEPTH_READ);
		ResourceStateTracker::Get()->TransiationResource( m_HZBBuffer->M_HZBTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &m_GBuffer->m_ColorDeferredCpuHandle, true, NULL);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ReflectionEnvironmentPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ReflectionEnvironmentBuffer->m_Buffer->GetGpuHandle()), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderPostProcess()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Post Process" );

		PostProcess_TemporalAA();
		PostProcess_Tonemapping();

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_TemporalAA()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "TAA" );

		m_TAABuffer->MapBuffer(m_CommandList->GetD3D12CommandList(), this);

		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_ColorDeferredTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_DepthTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_GBuffer->m_VelocityTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_TAABuffer->GetHistoryResource(m_SceneView.FrameIndex)->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_TAABuffer->GetFrameResource(m_SceneView.FrameIndex)->GetD3D12Resource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		m_TAABuffer->Bind( m_CommandList->GetD3D12CommandList() );

		m_CommandList->GetD3D12CommandList()->SetComputeRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_TAAPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_TAABuffer->m_Buffer->GetGpuHandle()), 1);
		m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		int32 DispatchSizeX = m_SceneView.Size.X / 8 + 1;
		int32 DispatchSizeY = m_SceneView.Size.Y / 8 + 1;
		m_CommandList->GetD3D12CommandList()->Dispatch(DispatchSizeX, DispatchSizeY, 1);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_Tonemapping()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Tone mapping" );

		m_TonemapBuffer->Bind(m_CommandList->GetD3D12CommandList());
		
		ResourceStateTracker::Get()->TransiationResource( m_TAABuffer->GetFrameResource(m_SceneView.FrameIndex)->GetD3D12Resource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_TonemapBuffer->m_TonemapTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_TonemapPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_TAABuffer->GetFrameSRV(m_SceneView.FrameIndex).GetIndex() , 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

#if WITH_EDITOR
	void SceneRenderer::RenderEditorPrimitives()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Editor Primitives" );

		ID3D12Resource* GBufferDepth = m_GBuffer->m_DepthTarget->GetD3D12Resource();
		ID3D12Resource* EditorPrimitiveDepth = m_EditorPrimitiveBuffer->m_DepthTarget->GetD3D12Resource();

		ResourceStateTracker::Get()->TransiationResource( GBufferDepth, D3D12_RESOURCE_STATE_COPY_SOURCE );
		ResourceStateTracker::Get()->TransiationResource( EditorPrimitiveDepth, D3D12_RESOURCE_STATE_COPY_DEST );

		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		m_CommandList->GetD3D12CommandList()->CopyResource(EditorPrimitiveDepth, GBufferDepth);

		ResourceStateTracker::Get()->TransiationResource( EditorPrimitiveDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE );
		ResourceStateTracker::Get()->TransiationResource( m_EditorPrimitiveBuffer->m_ColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());

		m_EditorPrimitiveBuffer->Clear(m_CommandList->GetD3D12CommandList());
		m_EditorPrimitiveBuffer->Bind(m_CommandList->GetD3D12CommandList());

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderEditorPrimitivePass(m_CommandList->GetD3D12CommandList(), this);
		}

// ------------------------------------------------------------------------------------------

		ID3D12Resource* EditorPrimitiveColor = m_EditorPrimitiveBuffer->m_ColorTarget->GetD3D12Resource();

		ResourceStateTracker::Get()->TransiationResource( EditorPrimitiveColor, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &m_TonemapBuffer->m_TonemapHandle, true, NULL);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get());
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_EditorPrimitiveBuffer->m_ColorTarget->GetGpuHandle()), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderEditorSelection()
	{
		SCOPE_STAT();
		
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Editor Selection" );

		ResourceStateTracker::Get()->TransiationResource( m_EditorSelectionBuffer->m_DepthStencilTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());

		m_EditorSelectionBuffer->Clear(m_CommandList->GetD3D12CommandList());
		m_EditorSelectionBuffer->Bind(m_CommandList->GetD3D12CommandList());

		for ( PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies )
		{
			Proxy->RenderSelectionPass( m_CommandList->GetD3D12CommandList(), this);
		}

		ResourceStateTracker::Get()->TransiationResource( m_EditorSelectionBuffer->m_DepthStencilTarget, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		ResourceStateTracker::Get()->TransiationResource( m_TonemapBuffer->m_TonemapTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets( 1, &m_TonemapBuffer->m_TonemapHandle, true, NULL );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ResolveEditorSelectionPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_BindlessViewBuffer[Renderer::Get()->GetCurrentBackbufferIndex()]->GetGpuHandle()), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_EditorSelectionBuffer->m_DepthStencilTarget->GetGpuHandle()), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(Renderer::Get()->m_StaticSamplersBuffer->GetGpuHandle()), 2);

		m_CommandList->GetD3D12CommandList()->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList->GetD3D12CommandList());

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

#endif

	void SceneRenderer::Render()
	{
		SCOPE_STAT();

		m_CommandList->SetAllocatorAndReset(Renderer::Get()->m_SwapChain->GetBackBufferIndex());
		Renderer::Get()->SetBindlessHeaps( m_CommandList->GetD3D12CommandList() );

		ResizeViewConditional();

#if WITH_EDITOR
		ProccessMousePickQueue();
#endif

		if (!m_RenderingEnabled )
		{
			m_CommandList->Close();
			return;
		}

		ResolvePostProcessSettings();
		RecalculateView();

		Renderer::Get()->SetBindlessHeaps(m_CommandList->GetD3D12CommandList());

#if WITH_EDITOR
		RenderHitProxyPass();
#endif

		RenderShadowDepths();
		RenderBasePass();
		RenderHZB();
		RenderAO();
		RenderLights();
		RenderSSR();
		RenderReflection();
		RenderPostProcess();

#if WITH_EDITOR
		RenderEditorPrimitives();
		RenderEditorSelection();

		{
			ResourceStateTracker::Get()->TransiationResource( m_TonemapBuffer->m_TonemapTarget, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
		}
#endif

		m_CommandList->Close();
	}

	ID3D12Resource* SceneRenderer::GetViewResource()
	{
		return m_TonemapBuffer->m_TonemapTarget->GetD3D12Resource();
	}

	void SceneRenderer::ResizeView( const IntPoint& InSize )
	{
		SCOPE_STAT();

		m_CachedRenderSize = IntPoint::ComponentWiseMax(InSize, IntPoint(1));
		m_RenderSize = m_CachedRenderSize;

		m_GBuffer->Resize( GetViewportSize() );
		m_HZBBuffer->Resize( GetViewportSize() );
		m_TonemapBuffer->Resize( GetViewportSize() );
		m_AOBuffer->Resize( GetViewportSize() );
		m_ScreenSpaceReflectionBuffer->Resize( GetViewportSize() );
		m_ReflectionEnvironmentBuffer->Resize( GetViewportSize() );
		m_TAABuffer->Resize( GetViewportSize() );

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
		SCOPE_STAT();

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
		SCOPE_STAT();

		m_SceneView.FrameIndex = ++m_FrameIndex;
		m_SceneView.FrameIndexMod8 = m_FrameIndex % 8;

		m_SceneView.Size = GetViewportSize();
		m_SceneView.InvSizeX = 1.0f / m_SceneView.Size.X;
		m_SceneView.InvSizeY = 1.0f / m_SceneView.Size.Y;

		m_SceneView.PrevJitterOffset[0] = m_SceneView.JitterOffset[0];
		m_SceneView.PrevJitterOffset[1] = m_SceneView.JitterOffset[1];

		m_SceneView.JitterOffset[0] = TAABuffer::m_JitterOffsets[m_SceneView.FrameIndexMod8][0] * m_SceneView.InvSizeX * m_PostProcessSettings->m_TAASettings.m_JitterOffsetScale;
		m_SceneView.JitterOffset[1] = TAABuffer::m_JitterOffsets[m_SceneView.FrameIndexMod8][1] * m_SceneView.InvSizeY * m_PostProcessSettings->m_TAASettings.m_JitterOffsetScale;

		ViewInfo VInfo = m_Scene->GetWorld()->GetPlayerWorldView();
		VInfo.AspectRatio = (float) GetViewportSize().X / GetViewportSize().Y;

		m_SceneView.AspectRatio = VInfo.AspectRatio;
		m_SceneView.WorldToView = VInfo.CalculateViewMatrix();
		m_SceneView.ViewToProjection = VInfo.CalculateProjectionMatrix();
		m_SceneView.ViewToProjection.m_Matrix.m[2][0] += m_SceneView.JitterOffset[0];
		m_SceneView.ViewToProjection.m_Matrix.m[2][1] += m_SceneView.JitterOffset[1];

		m_SceneView.WorldToProjection = m_SceneView.WorldToView * m_SceneView.ViewToProjection;
		m_SceneView.ProjectionToView = XMMatrixInverse( NULL, m_SceneView.ViewToProjection.Get() );
		m_SceneView.ViewToWorld = XMMatrixInverse(NULL, m_SceneView.WorldToView.Get());
		//m_SceneView.ProjectionToWorld = m_SceneView.ViewToWorld * m_SceneView.ProjectionToView;
		m_SceneView.ProjectionToWorld = m_SceneView.ProjectionToView * m_SceneView.ViewToWorld;

		Vector4 V1(1, 0, 0, 0);
		Vector4 V2(0, 1, 0, 0);
		Vector4 V3(0, 0, m_SceneView.ViewToProjection.m_Matrix.m[2][2], 1);
		Vector4 V4(0, 0, m_SceneView.ViewToProjection.m_Matrix.m[3][2], 0);
		Matrix M = Matrix(V1, V2, V3, V4);
		m_SceneView.ScreenToTranslatedWorld = M * m_SceneView.ProjectionToWorld;

		m_SceneView.LocalToCameraView = Matrix( Transform(VInfo.Location, VInfo.Rotation) ).Get() * m_SceneView.WorldToView.Get();		

		m_SceneView.CameraPos = VInfo.Location;
		m_SceneView.CameraDir = VInfo.Rotation.GetVector();

		m_SceneView.InvTanHalfFov = m_SceneView.ViewToProjection.m_Matrix.m[0][0];

		{
			float DepthMul = m_SceneView.ViewToProjection.m_Matrix.m[2][2];
			float DepthAdd = m_SceneView.ViewToProjection.m_Matrix.m[3][2];

			if (DepthAdd == 0.f)
			{
				DepthAdd = 0.00000001f;
			}

			bool bIsPerspectiveProjection = m_SceneView.ViewToProjection.m_Matrix.m[3][3] < 1.0f;

			if (bIsPerspectiveProjection)
			{
				float SubtractValue = DepthMul / DepthAdd;
				SubtractValue -= 0.00000001f;

				m_SceneView.InvDeviceZToWorldZTransform = Vector4(0.0f, 0.0f, 1.0f / DepthAdd, SubtractValue);
			}
			else
			{
				m_SceneView.InvDeviceZToWorldZTransform = Vector4(1.0f / m_SceneView.ViewToProjection.m_Matrix.m[2][2],
					-m_SceneView.ViewToProjection.m_Matrix.m[3][2] / m_SceneView.ViewToProjection.m_Matrix.m[2][2] + 1.0f, 0.0f, 1.0f);
			}
		}

		UpdateViewBuffer();
	}

#if WITH_EDITOR
	// TODO: support box selection
	void SceneRenderer::QueueMousePickEvent( const IntPoint& ScreenPosition )
	{
		m_MousePickQueue.emplace_back( ScreenPosition );
	}

	void SceneRenderer::ProccessMousePickQueue()
	{
		SCOPE_STAT();

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
				if (Event.FenceValue <= m_MousePickFence->GetCompletedValue())
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
				CD3DX12_RESOURCE_DESC::Buffer( 16 ), D3D12_RESOURCE_STATE_COPY_DEST, false);

#if D3D12_Debug_INFO
			Event.ReadbackBuffer->SetName("ScreenPickReadbackBuffer");
#endif

			ResourceStateTracker::Get()->TransiationResource(m_HitProxyRenderBuffer->m_GuidTarget->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_SOURCE);

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

			ResourceStateTracker::Get()->FlushResourceBarriers(m_CommandList->GetD3D12CommandList());
			m_CommandList->GetD3D12CommandList()->CopyTextureRegion( &DestLoc, 0, 0, 0, &SourceLoc, &CopyBox );

			// push a fence
			Renderer::Get()->GetCommandQueue()->Signal( m_MousePickFence.Get(), ++m_FenceValue );
			Event.FenceValue = m_FenceValue;

			Event.Initalized = true;
		}

	}

#endif

}