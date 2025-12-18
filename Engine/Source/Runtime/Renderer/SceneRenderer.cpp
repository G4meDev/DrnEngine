#include "DrnPCH.h"
#include "SceneRenderer.h"

#include "Runtime/Engine/LightSceneProxy.h"
#include "Runtime/Engine/DecalSceneProxy.h"
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
#include "Runtime/Renderer/RenderBuffer/SceneDownSampleBuffer.h"
#include "Runtime/Renderer/RenderBuffer/BloomBuffer.h"
#include "Runtime/Renderer/RenderBuffer/DecalBuffer.h"

#include "Runtime/Engine/PostProcessVolume.h"

#include "Editor/Thumbnail/ThumbnailManager.h"

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
		m_CommandList = nullptr;

#if WITH_EDITOR
		if (m_MousePickQueue.size() > 0 || m_ScreenReprojectionQueue.size() > 0)
		{
			LOG(LogSceneRenderer, Warning, "mouse pick event still has %i events", (int)m_MousePickQueue.size());
			__debugbreak();
		}

		drn_check( ThumbnailCaptureEvents.size() == 0 );

		OnSceneRendererDestroy.Braodcast();
#endif
	}

	void SceneRenderer::Init()
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		m_CommandList = new D3D12CommandList(Renderer::Get()->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT, NUM_BACKBUFFERS, m_Name);
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

		m_SceneDownSampleBuffer = std::make_shared<class SceneDownSampleBuffer>();
		m_SceneDownSampleBuffer->Init();

		m_BloomBuffer = std::make_shared<class BloomBuffer>();
		m_BloomBuffer->Init();

		m_DecalBuffer = std::make_shared<class DecalBuffer>();
		m_DecalBuffer->Init();

#if WITH_EDITOR
		// mouse picking components
		m_HitProxyRenderBuffer = std::make_shared<class HitProxyRenderBuffer>();
		m_HitProxyRenderBuffer->Init();

		m_EditorPrimitiveBuffer = std::make_shared<class EditorPrimitiveRenderBuffer>();
		m_EditorPrimitiveBuffer->Init();

		m_EditorSelectionBuffer = std::make_shared<class EditorSelectionRenderBuffer>();
		m_EditorSelectionBuffer->Init();
#endif

		ResizeView(IntPoint(1920, 1080));

		m_RenderTask.emplace( [&]() { Render(); } );

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
		ViewBuffer = RenderUniformBuffer::Create(m_CommandList->GetParentDevice(), sizeof(SceneRendererView), EUniformBufferUsage::SingleFrame, &m_SceneView);
	}

	void SceneRenderer::RenderPrepass()
	{
		SCOPE_STAT();

		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "Prepass");

		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_CommandList->FlushBarriers();

		m_GBuffer->ClearDepth( m_CommandList);
		m_GBuffer->BindDepth(m_CommandList);

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderPrePass(m_CommandList, this);
		}

		PIXEndEvent(m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderShadowDepths()
	{
		SCOPE_STAT();

		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "ShadowDepths");

		for ( LightSceneProxy* Proxy : m_Scene->m_LightProxies )
		{
			Proxy->RenderShadowDepth(m_CommandList, this);
		}

		PIXEndEvent(m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderDecals()
	{
		SCOPE_STAT();
		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "Deferred Decals");

		m_CommandList->TransitionResourceWithTracking(m_DecalBuffer->m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_DecalBuffer->m_NormalTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_DecalBuffer->m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->FlushBarriers();

		m_DecalBuffer->Clear(m_CommandList);
		m_DecalBuffer->Bind(m_CommandList);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get()); // TODO: remove and set at start of render

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 6);

		for ( DecalSceneProxy* Proxy : m_Scene->m_DecalProxies )
		{
			Proxy->Render(m_CommandList, this);
		}

// -----------------------------------------------------------------------------------------------------------------------------------------------------

		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_DEPTH_READ);
		m_CommandList->FlushBarriers();

		m_CommandList->SetViewport( 0, 0, 0, m_RenderSize.X, m_RenderSize.Y, 1 );

		D3D12_CPU_DESCRIPTOR_HANDLE const RenderTargets[3] = 
		{
			m_DecalBuffer->m_BaseColorTarget->GetRenderTargetView()->GetView(),
			m_DecalBuffer->m_NormalTarget->GetRenderTargetView()->GetView(),
			m_DecalBuffer->m_MasksTarget->GetRenderTargetView()->GetView()
		};

		D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle = m_GBuffer->m_DepthTarget->GetDepthStencilView(EDepthStencilViewType::DepthWrite)->GetView();
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets( 3, RenderTargets, false, &DepthHandle);

		for ( PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies )
		{
			Proxy->RenderDecalPass(m_CommandList, this);
		}

		PIXEndEvent(m_CommandList->GetD3D12CommandList());
	}

#if WITH_EDITOR
	void SceneRenderer::RenderHitProxyPass()
	{
		SCOPE_STAT();

		PIXBeginEvent(m_CommandList->GetD3D12CommandList(), 1, "HitProxy");

		m_CommandList->TransitionResourceWithTracking(m_HitProxyRenderBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_CommandList->TransitionResourceWithTracking(m_HitProxyRenderBuffer->m_GuidTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->FlushBarriers();

		m_HitProxyRenderBuffer->Clear( m_CommandList);
		m_HitProxyRenderBuffer->Bind(m_CommandList);

		const bool IsGameMode = GetScene()->GetWorld()->IsInGameMode();

		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			if (!IsGameMode || (IsGameMode && !Proxy->IsEditorPrimitive()))
			{
				Proxy->RenderHitProxyPass(m_CommandList, this);
			}
		}

		PIXEndEvent(m_CommandList->GetD3D12CommandList());
	}
#endif

	void SceneRenderer::RenderBasePass()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "BasePass" );

		m_CommandList->TransitionResourceWithTracking(m_DecalBuffer->m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_DecalBuffer->m_NormalTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_DecalBuffer->m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_DEPTH_READ);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_ColorDeferredTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_WorldNormalTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_MasksBTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_VelocityTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->FlushBarriers();

		m_GBuffer->Clear( m_CommandList);
		m_GBuffer->Bind(m_CommandList);

		if (m_DecalBuffer)
		{
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_DecalBuffer->m_BaseColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 7);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_DecalBuffer->m_NormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 8);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_DecalBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 9);
		}


		for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
		{
			Proxy->RenderMainPass(m_CommandList, this);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderHZB()
	{
		SCOPE_STAT();
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "HZB" );

		m_CommandList->TransitionResourceWithTracking(m_HZBBuffer->M_HZBTarget->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->FlushBarriers();

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

			uint32 ParentViewIndex = DispatchStartMipIndex == 0 ?
				m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex() :
				m_HZBBuffer->m_SRVHandles[DispatchStartMipIndex / 4 - 1]->GetDescriptorHeapIndex();

			if ( DispatchStartMipIndex != 0)
			{
				m_CommandList->TransitionResourceWithTracking(m_HZBBuffer->M_HZBTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, DispatchStartMipIndex - 1);
				m_CommandList->FlushBarriers();
			}

			ID3D12PipelineState* DispatchPSO = nullptr;

			if		( DispatchMipCount == 4)	{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_4Mip_PSO; }
			else if ( DispatchMipCount == 3)	{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_3Mip_PSO; }
			else if ( DispatchMipCount == 2)	{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_2Mip_PSO; }
			else 								{ DispatchPSO = CommonResources::Get()->m_HZBPSO->m_1Mip_PSO; }

			m_CommandList->GetD3D12CommandList()->SetComputeRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
			m_CommandList->GetD3D12CommandList()->SetPipelineState(DispatchPSO);

			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, ParentViewIndex, 1);
			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			Vector4 DispatchIDToUV;
			Vector InvSize = Vector(1.0f / MipSize.X, 1.0f / MipSize.Y, 0);

			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstants(0, 4, &DispatchIDToUV, 8);
			m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstants(0, 3, &InvSize, 12);

			for (int32 i = 0; i < DispatchMipCount; i++)
			{
				const int32 MipIndex = DispatchStartMipIndex + i;
				m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, m_HZBBuffer->m_UAVHandles[MipIndex]->GetDescriptorHeapIndex(), OutputIndexStart + i);
			}

			m_CommandList->GetD3D12CommandList()->Dispatch(DispatchSize.X, DispatchSize.Y, 1);
 		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderAO()
	{
		SCOPE_STAT();
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "AO" );

		m_AOBuffer->MapBuffer(m_CommandList, this, m_PostProcessSettings->m_SSAOSettings);

		m_CommandList->TransitionResourceWithTracking( m_HZBBuffer->M_HZBTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_AOBuffer->m_AOSetupTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->TransitionResourceWithTracking( m_AOBuffer->m_AOHalfTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->TransitionResourceWithTracking( m_AOBuffer->m_AOTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_WorldNormalTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->FlushBarriers();

		{
			m_AOBuffer->BindSetup(m_CommandList);

			m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_AmbientOcclusionPSO->m_SetupPSO );

			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_AOBuffer->AoBuffer->GetViewIndex(), 1);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
		}

		{
			m_CommandList->TransitionResourceWithTracking( m_AOBuffer->m_AOSetupTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
			m_CommandList->FlushBarriers();

			m_AOBuffer->BindHalf(m_CommandList);

			m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_AmbientOcclusionPSO->m_HalfPSO );


			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_AOBuffer->AoBuffer->GetViewIndex(), 1);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
		}

		{
			m_CommandList->TransitionResourceWithTracking( m_AOBuffer->m_AOHalfTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
			m_CommandList->FlushBarriers();

			m_AOBuffer->BindMain(m_CommandList);

			m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_AmbientOcclusionPSO->m_MainPSO );

			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_AOBuffer->AoBuffer->GetViewIndex(), 1);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderLights()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "LightPass" );

		m_CommandList->TransitionResourceWithTracking( m_AOBuffer->m_AOTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->FlushBarriers();

		m_CommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		m_CommandList->GetD3D12CommandList()->SetPipelineState(CommonResources::Get()->m_LightPassPSO->m_PSO);

		m_GBuffer->BindLightPass(m_CommandList);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		// 1 is light buffer
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_GBuffer->m_BaseColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 3);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_GBuffer->m_WorldNormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 4);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_GBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 5);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 6);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_AOBuffer->m_AOTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 8);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_GBuffer->m_MasksBTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 9);

		for ( LightSceneProxy* Proxy : m_Scene->m_LightProxies )
		{
			Proxy->Render(m_CommandList, this);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList());
	}

	void SceneRenderer::RenderSSR()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Screen Space Reflection" );

		m_ScreenSpaceReflectionBuffer->MapBuffer(m_CommandList, this, m_PostProcessSettings->m_SSRSettings);

		m_CommandList->TransitionResourceWithTracking( m_HZBBuffer->M_HZBTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_ScreenSpaceReflectionBuffer->m_Target->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_ColorDeferredTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_WorldNormalTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->FlushBarriers();

		m_ScreenSpaceReflectionBuffer->Bind(m_CommandList);
		m_ScreenSpaceReflectionBuffer->Clear(m_CommandList);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ScreenSpaceReflectionPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_ScreenSpaceReflectionBuffer->Buffer->GetViewIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

		m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderReflection()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Reflection Environment" );

		m_ReflectionEnvironmentBuffer->MapBuffer(m_CommandList, this);
		
		m_CommandList->TransitionResourceWithTracking( m_HZBBuffer->M_HZBTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_ColorDeferredTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->TransitionResourceWithTracking(m_ScreenSpaceReflectionBuffer->m_Target->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_BaseColorTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_WorldNormalTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_MasksTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_AOBuffer->m_AOTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->FlushBarriers();

		D3D12_CPU_DESCRIPTOR_HANDLE DeferredColorHandle = m_GBuffer->m_ColorDeferredTarget->GetRenderTargetView( 0, 0 )->GetView();
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &DeferredColorHandle, true, NULL);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ReflectionEnvironmentPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_ReflectionEnvironmentBuffer->Buffer->GetViewIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

		m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderPostProcess()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Post Process" );

		PostProcess_TemporalAA();
		PostProcess_SceneDownSample();
		PostProcess_Bloom();
		PostProcess_Tonemapping();

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_TemporalAA()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "TAA" );

		m_TAABuffer->MapBuffer(m_CommandList, this);

		m_CommandList->TransitionResourceWithTracking( m_TAABuffer->GetHistoryResource(m_SceneView.FrameIndex)->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking( m_TAABuffer->GetFrameResource(m_SceneView.FrameIndex)->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_ColorDeferredTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_VelocityTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking( m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->FlushBarriers();

		m_TAABuffer->Bind(m_CommandList);

		m_CommandList->GetD3D12CommandList()->SetComputeRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_TAAPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, m_TAABuffer->Buffer->GetViewIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetComputeRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

		int32 DispatchSizeX = m_SceneView.Size.X / 8 + 1;
		int32 DispatchSizeY = m_SceneView.Size.Y / 8 + 1;
		m_CommandList->GetD3D12CommandList()->Dispatch(DispatchSizeX, DispatchSizeY, 1);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_SceneDownSample()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "SceneDownSample" );

		m_SceneDownSampleBuffer->MapBuffer(m_CommandList, this);

		m_CommandList->TransitionResourceWithTracking( m_TAABuffer->GetFrameResource(m_SceneView.FrameIndex)->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking( m_SceneDownSampleBuffer->m_DownSampleTargets[0]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->FlushBarriers();

		D3D12_CPU_DESCRIPTOR_HANDLE MainHandle = m_SceneDownSampleBuffer->m_DownSampleTargets[0]->GetRenderTargetView()->GetView();
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &MainHandle, true, NULL);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_SceneDownSamplePSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_SceneDownSampleBuffer->Buffer[0]->GetViewIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

		m_CommandList->SetViewport(0 ,0, 0, m_SceneDownSampleBuffer->m_Viewports[0].X, m_SceneDownSampleBuffer->m_Viewports[0].Y, 1);

		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);

		for (int32 i = 1; i < NUM_SCENE_DOWNSAMPLES; i++)
		{
			m_CommandList->TransitionResourceWithTracking( m_SceneDownSampleBuffer->m_DownSampleTargets[i - 1]->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			m_CommandList->TransitionResourceWithTracking( m_SceneDownSampleBuffer->m_DownSampleTargets[i]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_CommandList->FlushBarriers();

			D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_SceneDownSampleBuffer->m_DownSampleTargets[i]->GetRenderTargetView()->GetView();
			m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &Handle, true, NULL);

			//m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
			//m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_SceneDownSamplePSO->m_PSO );

			//m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
			m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_SceneDownSampleBuffer->Buffer[i]->GetViewIndex(), 1);
			//m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

			m_CommandList->SetViewport(0 ,0, 0, m_SceneDownSampleBuffer->m_Viewports[i].X, m_SceneDownSampleBuffer->m_Viewports[i].Y, 1);
			CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_Bloom()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Bloom" );

		m_BloomBuffer->MapBuffer(m_CommandList, this);

		for (int32 i = NUM_SCENE_DOWNSAMPLES - 1; i > -1; i--)
		{
			{
				m_CommandList->TransitionResourceWithTracking(m_SceneDownSampleBuffer->m_DownSampleTargets[i]->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
				m_CommandList->TransitionResourceWithTracking(m_BloomBuffer->m_BloomTargets[i * 2]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
				m_CommandList->FlushBarriers();

				D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_BloomBuffer->m_BloomTargets[i * 2]->GetRenderTargetView()->GetView();
				m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &Handle, true, NULL);

				m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
				m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_BloomPSO->m_BloomYPSO );

				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_BloomBuffer->Buffer[i * 2]->GetViewIndex(), 1);
				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				m_CommandList->SetViewport(0 ,0, 0, m_SceneDownSampleBuffer->m_Viewports[i].X, m_SceneDownSampleBuffer->m_Viewports[i].Y, 1);
				CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
			}

			{
				const bool FirstChain = i == NUM_SCENE_DOWNSAMPLES - 1;

				m_CommandList->TransitionResourceWithTracking(m_BloomBuffer->m_BloomTargets[i * 2]->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
				m_CommandList->TransitionResourceWithTracking(m_BloomBuffer->m_BloomTargets[i * 2 + 1]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
				if (!FirstChain)
				{
					m_CommandList->TransitionResourceWithTracking(m_BloomBuffer->m_BloomTargets[(i + 1) * 2 + 1]->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
				}
				m_CommandList->FlushBarriers();

				D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_BloomBuffer->m_BloomTargets[i * 2 + 1]->GetRenderTargetView()->GetView();
				m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &Handle, true, NULL);

				m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
				m_CommandList->GetD3D12CommandList()->SetPipelineState( FirstChain ? CommonResources::Get()->m_BloomPSO->m_BloomXPSO : CommonResources::Get()->m_BloomPSO->m_BloomXAddtivePSO );

				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_BloomBuffer->Buffer[i * 2 + 1]->GetViewIndex(), 1);
				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				m_CommandList->SetViewport(0 ,0, 0, m_SceneDownSampleBuffer->m_Viewports[i].X, m_SceneDownSampleBuffer->m_Viewports[i].Y, 1);
				CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
			}
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::PostProcess_Tonemapping()
	{
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Tone mapping" );

		m_TonemapBuffer->Bind(m_CommandList);
		
		m_CommandList->TransitionResourceWithTracking(m_TAABuffer->GetFrameResource(m_SceneView.FrameIndex)->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_BloomBuffer->m_BloomTargets[1]->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking(m_TonemapBuffer->m_TonemapTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->FlushBarriers();

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_TonemapPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_TAABuffer->GetFrameResource(m_SceneView.FrameIndex)->GetShaderResourceView()->GetDescriptorHeapIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_BloomBuffer->m_BloomTargets[1]->GetShaderResourceView()->GetDescriptorHeapIndex(), 3);

		m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

#if WITH_EDITOR
	void SceneRenderer::RenderEditorPrimitives()
	{
		SCOPE_STAT();

		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Editor Primitives" );

		ID3D12Resource* GBufferDepth = m_GBuffer->m_DepthTarget->GetResource()->GetResource();
		ID3D12Resource* EditorPrimitiveDepth = m_EditorPrimitiveBuffer->m_DepthTarget->GetResource()->GetResource();

		m_CommandList->TransitionResourceWithTracking(m_EditorPrimitiveBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST);
		m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_CommandList->FlushBarriers();

		m_CommandList->GetD3D12CommandList()->CopyResource(EditorPrimitiveDepth, GBufferDepth);
		
		m_CommandList->TransitionResourceWithTracking(m_EditorPrimitiveBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_CommandList->TransitionResourceWithTracking(m_EditorPrimitiveBuffer->m_ColorTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->FlushBarriers();

		m_EditorPrimitiveBuffer->Clear(m_CommandList);
		m_EditorPrimitiveBuffer->Bind(m_CommandList);

		if (!GetScene()->GetWorld()->IsInGameMode())
		{
			for (PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies)
			{
				Proxy->RenderEditorPrimitivePass(m_CommandList, this);
			}
		}

// ------------------------------------------------------------------------------------------

		ID3D12Resource* EditorPrimitiveColor = m_EditorPrimitiveBuffer->m_ColorTarget->GetResource()->GetResource();

		m_CommandList->TransitionResourceWithTracking(m_EditorPrimitiveBuffer->m_ColorTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->FlushBarriers();

		D3D12_CPU_DESCRIPTOR_HANDLE TonemapHandle = m_TonemapBuffer->m_TonemapTarget->GetRenderTargetView()->GetView();
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &TonemapHandle, true, NULL);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get());
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ResolveAlphaBlendedPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_EditorPrimitiveBuffer->m_ColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

		m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::RenderEditorSelection()
	{
		SCOPE_STAT();
		
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Editor Selection" );

		m_CommandList->TransitionResourceWithTracking(m_EditorSelectionBuffer->m_DepthStencilTarget->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_CommandList->FlushBarriers();

		m_EditorSelectionBuffer->Clear(m_CommandList);
		m_EditorSelectionBuffer->Bind(m_CommandList);

		const bool IsGameMode = GetScene()->GetWorld()->IsInGameMode();

		for ( PrimitiveSceneProxy* Proxy : m_Scene->m_PrimitiveProxies )
		{
			if (!IsGameMode || (IsGameMode && !Proxy->IsEditorPrimitive()) )
			{
				Proxy->RenderSelectionPass( m_CommandList, this);
			}
		}

		m_CommandList->TransitionResourceWithTracking(m_EditorSelectionBuffer->m_DepthStencilTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		m_CommandList->TransitionResourceWithTracking( m_TonemapBuffer->m_TonemapTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_CommandList->FlushBarriers();

		D3D12_CPU_DESCRIPTOR_HANDLE TonemapHandle = m_TonemapBuffer->m_TonemapTarget->GetRenderTargetView()->GetView();
		m_CommandList->GetD3D12CommandList()->OMSetRenderTargets(1, &TonemapHandle, true, NULL);

		m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		m_CommandList->GetD3D12CommandList()->SetPipelineState( CommonResources::Get()->m_ResolveEditorSelectionPSO->m_PSO );

		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, m_EditorSelectionBuffer->m_StencilView->GetDescriptorHeapIndex(), 1);
		m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

		m_CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	void SceneRenderer::GetResourcesForBufferVisualization(EBufferVisualization BufferVisualization, RenderResource*& OutResource, uint32& OutTextureIndex )
	{
		switch ( BufferVisualization )
		{
		case EBufferVisualization::BaseColor: OutResource = m_GBuffer->m_BaseColorTarget->GetResource(); OutTextureIndex = m_GBuffer->m_BaseColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;

		case EBufferVisualization::Metallic:
		case EBufferVisualization::Roughness:
		case EBufferVisualization::MaterialAO:
		case EBufferVisualization::ShadingModel: OutResource = m_GBuffer->m_MasksTarget->GetResource(); OutTextureIndex = m_GBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;

		case EBufferVisualization::PreTonemapColor: OutResource = m_GBuffer->m_ColorDeferredTarget->GetResource(); OutTextureIndex = m_GBuffer->m_ColorDeferredTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;
		case EBufferVisualization::WorldNormal: OutResource = m_GBuffer->m_WorldNormalTarget->GetResource(); OutTextureIndex = m_GBuffer->m_WorldNormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;
		case EBufferVisualization::SubsurfaceColor: OutResource = m_GBuffer->m_MasksBTarget->GetResource(); OutTextureIndex = m_GBuffer->m_MasksBTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;
		case EBufferVisualization::Depth:
		case EBufferVisualization::LinearDepth: OutResource = m_GBuffer->m_DepthTarget->GetResource(); OutTextureIndex = m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;

		case EBufferVisualization::ScreenSpaceAO: OutResource = m_AOBuffer->m_AOTarget->GetResource(); OutTextureIndex = m_AOBuffer->m_AOTarget->GetShaderResourceView()->GetDescriptorHeapIndex(); break;
		case EBufferVisualization::Bloom: OutResource = m_BloomBuffer->m_BloomTargets[1]->GetResource(); OutTextureIndex = m_BloomBuffer->m_BloomTargets[1]->GetShaderResourceView()->GetDescriptorHeapIndex(); break;
		case EBufferVisualization::ScreenSpaceReflection: OutResource = m_ScreenSpaceReflectionBuffer->m_Target->GetResource(); OutTextureIndex = m_ScreenSpaceReflectionBuffer->m_Target->GetShaderResourceView()->GetDescriptorHeapIndex(); break;

		case EBufferVisualization::FinalImage:
		default: OutResource = nullptr; OutTextureIndex = 0;
		}
	}

	void SceneRenderer::RenderBufferVisulization()
	{
		SCOPE_STAT();
		
		PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Buffer Visualization" );

		World* OwningWorld = m_Scene ? m_Scene->GetWorld() : nullptr;
		if (OwningWorld && OwningWorld->GetBufferVisualization() != EBufferVisualization::FinalImage)
		{
			EBufferVisualization BufferVisulization = m_Scene->GetWorld()->GetBufferVisualization();
			RenderResource* VisualizationResource = nullptr;
			uint32 InputTextureIndex;

			GetResourcesForBufferVisualization(BufferVisulization, VisualizationResource, InputTextureIndex);
			ID3D12PipelineState* PSO = CommonResources::Get()->m_BufferVisualizerPSO->GetPSOForBufferVisualizer(BufferVisulization);

			if (VisualizationResource && PSO)
			{
				m_CommandList->TransitionResourceWithTracking(VisualizationResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
				m_CommandList->FlushBarriers();

				m_TonemapBuffer->Bind( m_CommandList );

				m_CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
				m_CommandList->GetD3D12CommandList()->SetPipelineState(PSO);

				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, ViewBuffer->GetViewIndex(), 0);
				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, InputTextureIndex, 1);
				m_CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);

				CommonResources::Get()->m_ScreenTriangle->BindAndDraw(m_CommandList);
			}
		}

		PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

#endif

	void SceneRenderer::Render()
	{
		SCOPE_STAT();


		m_CommandList->SetAllocatorAndReset(Renderer::Get()->m_SwapChain->GetBackBufferIndex());
		Renderer::Get()->SetBindlessHeaps( m_CommandList->GetD3D12CommandList() );
		m_CommandList->ClearState();

		//PIXBeginEvent( m_CommandList->GetD3D12CommandList(), 1, "Scene" );

		ResizeViewConditional();

#if WITH_EDITOR
		ProccessMousePickQueue();
		ProccessScreenReprojectionQueue();
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

		RenderPrepass();
		RenderShadowDepths();
		RenderDecals();
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
		RenderBufferVisulization();

		ProccessThumbnailCapture();

		{
			m_CommandList->TransitionResourceWithTracking(m_TonemapBuffer->m_TonemapTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			m_CommandList->FlushBarriers();
		}
#endif

		m_CommandList->Close();

		//PIXEndEvent( m_CommandList->GetD3D12CommandList() );
	}

	ID3D12Resource* SceneRenderer::GetViewResource()
	{
		return m_TonemapBuffer->m_TonemapTarget->GetResource()->GetResource();
	}

	void SceneRenderer::ResizeView( const IntPoint& InSize )
	{
		SCOPE_STAT();

		LOG( LogSceneRenderer, Info, "Resize from \"%s\" to \"%s\".", m_CachedRenderSize.ToString().c_str(), m_RenderSize.ToString().c_str() );

		m_CachedRenderSize = IntPoint::ComponentWiseMax(InSize, IntPoint(1));
		m_RenderSize = m_CachedRenderSize;

		m_GBuffer->Resize( GetViewportSize() );
		m_HZBBuffer->Resize( GetViewportSize() );
		m_TonemapBuffer->Resize( GetViewportSize() );
		m_AOBuffer->Resize( GetViewportSize() );
		m_ScreenSpaceReflectionBuffer->Resize( GetViewportSize() );
		m_ReflectionEnvironmentBuffer->Resize( GetViewportSize() );
		m_TAABuffer->Resize( GetViewportSize() );
		m_SceneDownSampleBuffer->Resize( GetViewportSize() );
		m_BloomBuffer->Resize( GetViewportSize() );
		m_DecalBuffer->Resize( GetViewportSize() );

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

		m_SceneView.JitterOffset[0] = TAABuffer::m_JitterOffsets[m_SceneView.FrameIndexMod8].GetX() * m_SceneView.InvSizeX * m_PostProcessSettings->m_TAASettings.m_JitterOffsetScale;
		m_SceneView.JitterOffset[1] = TAABuffer::m_JitterOffsets[m_SceneView.FrameIndexMod8].GetY() * m_SceneView.InvSizeY * m_PostProcessSettings->m_TAASettings.m_JitterOffsetScale;

		Matrix PrevViewMatrix = m_SceneView.WorldToView;
		Matrix PrevProjectionMatrix = m_SceneView.ViewToProjection;

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

		{
			Matrix ProjectionMatrixNoAA = m_SceneView.ViewToProjection;
			ProjectionMatrixNoAA.m_Matrix.m[2][0] -= m_SceneView.JitterOffset[0];
			ProjectionMatrixNoAA.m_Matrix.m[2][1] -= m_SceneView.JitterOffset[1];

			Matrix PrevProjectionMatrixNoAA = PrevProjectionMatrix;
			PrevProjectionMatrixNoAA.m_Matrix.m[2][0] -= m_SceneView.PrevJitterOffset[0];
			PrevProjectionMatrixNoAA.m_Matrix.m[2][1] -= m_SceneView.PrevJitterOffset[1];

			Matrix InvViewProjection = ProjectionMatrixNoAA.Inverse() * m_SceneView.ViewToWorld;
			Matrix Prev_ViewMatrix = PrevViewMatrix * PrevProjectionMatrixNoAA;

			m_SceneView.ClipToPreviousClip = InvViewProjection * Prev_ViewMatrix;
		}

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
	void SceneRenderer::ProccessThumbnailCapture()
	{
		while (ThumbnailCaptureEvents.size() > 0 && ThumbnailManager::Get()->IsCaptureThumbnailAllowed())
		{
			//Renderer::Get()->MarkFrameForCapture();

			ThumbnailCaptureEvent* Event = ThumbnailCaptureEvents.back();
			Event->bInitalized = true;
			ThumbnailCaptureEvents.pop_back();

			IntPoint ViewportSize = GetViewportSize();
			RenderResourceCreateInfo CreateInfo( "ThumbnailReadbackBuffer" );
			Event->ReadbackBuffer = RenderTexture2D::Create(m_CommandList, ViewportSize.X, ViewportSize.Y, DISPLAY_OUTPUT_FORMAT, 1, 1, false, ETextureCreateFlags::CPUReadback, CreateInfo );

			CopyTextureInfo CopyInfo;
			CopyInfo.Size = IntVector(ViewportSize.X, ViewportSize.Y, 0);

			m_CommandList->CopyTexture(m_TonemapBuffer->m_TonemapTarget, Event->ReadbackBuffer, CopyInfo);
			Event->FenceValue = Renderer::Get()->GetFence()->Signal();
		}
	}

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
				if (Renderer::Get()->GetFence()->IsFenceComplete(Event.FenceValue))
				{
					Guid Result;
					memcpy(&Result, Event.ReadbackBuffer->m_ResourceLocation.GetMappedBaseAddress(), sizeof(Guid));

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

			RenderResourceCreateInfo CreateInfo( "ScreenPickReadbackBuffer" );
			Event.ReadbackBuffer = RenderTexture2D::Create(m_CommandList, 1, 1, GBUFFER_GUID_FORMAT, 1, 1, false, ETextureCreateFlags::CPUReadback, CreateInfo );

			const IntPoint ClampedPos = IntPoint( std::clamp<int32>( Event.ScreenPos.X, 0, GetViewportSize().X - 1 ),
				std::clamp<int32>( Event.ScreenPos.Y, 0, GetViewportSize().Y - 1 ) );

			CopyTextureInfo CopyInfo;
			CopyInfo.Size = IntVector(1, 1, 0);
			CopyInfo.SourcePosition = IntVector(ClampedPos.X, ClampedPos.Y, 0);

			m_CommandList->CopyTexture(m_HitProxyRenderBuffer->m_GuidTarget, Event.ReadbackBuffer, CopyInfo);

			Event.FenceValue = Renderer::Get()->GetFence()->Signal();
			Event.Initalized = true;
		}
	}

	void SceneRenderer::ProccessScreenReprojectionQueue()
	{
		SCOPE_STAT();

		for ( auto it = m_ScreenReprojectionQueue.begin(); it != m_ScreenReprojectionQueue.end(); )
		{
			ScreenReprojectionEvent& Event = *it;

			if (!Event.Initalized)
			{
				KickstartScreenReprojection(Event);
				it++;
			}

			else
			{
				if (Renderer::Get()->GetFence()->IsFenceComplete(Event.FenceValue))
				{
					float Depth;
					memcpy(&Depth, Event.ReadbackBuffer->m_ResourceLocation.GetMappedBaseAddress(), sizeof(float));

					World* W = GetScene() ? GetScene()->GetWorld() : nullptr;
					if (W && Event.OnScreenReprojection.IsBound())
					{
						const bool bHit = Depth > 0.0f;
						Vector WorldPosition = Event.SceneView.PixelToWorld(Event.ScreenPos.X, Event.ScreenPos.Y, Depth);
						
						Event.OnScreenReprojection.Execute(bHit, WorldPosition, Event.Payload);
					}

					it = m_ScreenReprojectionQueue.erase(it);
				}

				else
				{
					it++;
				}
			}
		}
	}

	void SceneRenderer::KickstartScreenReprojection( ScreenReprojectionEvent& Event )
	{
		if ( m_GBuffer && m_GBuffer->m_DepthTarget)
		{
			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			RenderResourceCreateInfo CreateInfo( "ReprojectionReadbackBuffer" );
			Event.ReadbackBuffer = RenderTexture2D::Create(m_CommandList, 1, 1, DEPTH_FORMAT, 1, 1, false, ETextureCreateFlags::CPUReadback, CreateInfo );

			const IntPoint ClampedPos = IntPoint( std::clamp<int32>( Event.ScreenPos.X, 0, GetViewportSize().X - 1 ),
				std::clamp<int32>( Event.ScreenPos.Y, 0, GetViewportSize().Y - 1 ) );

			CopyTextureInfo CopyInfo;
			CopyInfo.Size = IntVector(1, 1, 0);
			CopyInfo.SourcePosition = IntVector(ClampedPos.X, ClampedPos.Y, 0);

			m_CommandList->CopyTexture(m_GBuffer->m_DepthTarget, Event.ReadbackBuffer, CopyInfo);

			Event.FenceValue = Renderer::Get()->GetFence()->Signal();
			Event.Initalized = true;
			Event.SceneView = GetSceneView();
		}
	}

#endif



}