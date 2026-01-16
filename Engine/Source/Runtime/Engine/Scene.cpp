#include "DrnPCH.h"
#include "Scene.h"

#include "Runtime/Engine/LightSceneProxy.h"
#include "Runtime/Engine/SkyLightSceneProxy.h"
#include "Runtime/Engine/PostProcessVolume.h"
#include "Runtime/Engine/DecalSceneProxy.h"

#include "Runtime/Engine/ReflectionCaptureComponent.h"

LOG_DEFINE_CATEGORY( LogScene, "Scene" );

namespace Drn
{
	Scene::Scene( World* InWorld )
		: m_World(InWorld)
	{
	}

	Scene::~Scene()
	{
		for (PrimitiveSceneProxy* Proxy : m_PrimitiveProxies)
		{
			delete Proxy;
		}
		for (PrimitiveSceneProxy* Proxy : m_PendingProxies)
		{
			delete Proxy;
		}

		for (LightSceneProxy* Proxy : m_PendingLightProxies)
		{
			delete Proxy;
		}

		for (LightSceneProxy* Proxy : m_LightProxies)
		{
			delete Proxy;
		}

// --------------------------------------------------------------------------------------

		for (SkyLightSceneProxy* Proxy : m_PendingSkyLightProxies)
		{
			delete Proxy;
		}

		for (SkyLightSceneProxy* Proxy : m_SkyLightProxies)
		{
			delete Proxy;
		}

// --------------------------------------------------------------------------------------

		for (DecalSceneProxy* Proxy : m_PendingDecalProxies)
		{
			Proxy->Release();
		}

		for (DecalSceneProxy* Proxy : m_DecalProxies)
		{
			Proxy->Release();
		}


// --------------------------------------------------------------------------------------

		for (auto it = m_SceneRenderers.begin(); it != m_SceneRenderers.end();)
		{
			SceneRenderer* SceneRen = *it;
			it = m_SceneRenderers.erase(it);
			SceneRen->Release();
		}
	}

	SceneRenderer* Scene::AllocateSceneRenderer()
	{
		SceneRenderer* NewSceneRenderer = new SceneRenderer(this);
		m_SceneRenderers.insert(NewSceneRenderer);

		return NewSceneRenderer;
	}

	void Scene::ReleaseSceneRenderer( SceneRenderer*& InSceneRenderer )
	{
		m_SceneRenderers.erase(InSceneRenderer);

		InSceneRenderer->Release();
		InSceneRenderer = nullptr;
	}

// ----------------------------------------------------------------------------

	void Scene::UpdatePendingProxyAndResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		std::erase_if(m_PrimitiveProxies, [CommandList](PrimitiveSceneProxy* Proxy)
		{
			drn_check(Proxy);

			if (Proxy->IsMarkedPendingKill())
			{
				delete Proxy;
				return true;
			}

			Proxy->UpdateResources(CommandList);
			return false;
		});

		for (auto it = m_PendingProxies.begin(); it != m_PendingProxies.end(); it++)
		{
			PrimitiveSceneProxy* Proxy = *it;
			drn_check(Proxy);

			if (Proxy->IsMarkedPendingKill())
			{
				delete Proxy;
			}
			else
			{
				Proxy->InitResources(CommandList);
				Proxy->UpdateResources(CommandList);
				m_PrimitiveProxies.push_back(Proxy);
			}
		}
		m_PendingProxies.clear();

		StaticPrimitiveMap.SetNumUninitialized(m_PrimitiveProxies.size());
		DynamicPrimitiveMap.SetNumUninitialized(m_PrimitiveProxies.size());

		for (int32 i = 0; i < m_PrimitiveProxies.size(); i++)
		{
			PrimitiveSceneProxy* Proxy = m_PrimitiveProxies[i];
			const bool bStatic = Proxy->IsStatic();

			StaticPrimitiveMap.SetBitNoCheck(i, bStatic);
			DynamicPrimitiveMap.SetBitNoCheck(i, !bStatic);
		}

// ----------------------------------------------------------------------------------

		std::erase_if( m_LightProxies, [CommandList](LightSceneProxy* Proxy)
		{
			drn_check(Proxy);

			if (Proxy->IsMarkedPendingKill())
			{
				delete Proxy;
				return true;
			}

			Proxy->UpdateResources(CommandList);
			return false;
		});

		for (auto it = m_PendingLightProxies.begin(); it != m_PendingLightProxies.end(); it++)
		{
			LightSceneProxy* Proxy = *it;
			drn_check(Proxy);

			if (Proxy->IsMarkedPendingKill())
			{
				delete Proxy;
			}
			else
			{
				Proxy->UpdateResources(CommandList);
				m_LightProxies.push_back(Proxy);
			}
		}
		m_PendingLightProxies.clear();

// ----------------------------------------------------------------------------------

		std::erase_if( m_SkyLightProxies, [CommandList](SkyLightSceneProxy* Proxy)
		{
			drn_check(Proxy);

			if (Proxy->IsMarkedPendingKill())
			{
				delete Proxy;
				return true;
			}

			Proxy->UpdateResources(CommandList);
			return false;
		});

		for (auto it = m_PendingSkyLightProxies.begin(); it != m_PendingSkyLightProxies.end(); it++)
		{
			SkyLightSceneProxy* Proxy = *it;
			drn_check(Proxy);

			if (Proxy->IsMarkedPendingKill())
			{
				delete Proxy;
			}
			else
			{
				Proxy->UpdateResources(CommandList);
				m_SkyLightProxies.push_back(Proxy);
			}
		}
		m_PendingSkyLightProxies.clear();

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingPostProcessProxies.begin(); it != m_PendingPostProcessProxies.end(); it++)
		{
			m_PostProcessProxies.insert(*it);
		}
		m_PendingPostProcessProxies.clear();

		for (auto it = m_PostProcessProxies.begin(); it != m_PostProcessProxies.end(); it++)
		{
			PostProcessSceneProxy* Proxy = *it;
			Proxy->UpdateResources();
		}

// ----------------------------------------------------------------------------------

		for (auto it = m_PendingDecalProxies.begin(); it != m_PendingDecalProxies.end(); it++)
		{
			m_DecalProxies.insert(*it);
		}
		m_PendingDecalProxies.clear();

		for (auto it = m_DecalProxies.begin(); it != m_DecalProxies.end(); it++)
		{
			DecalSceneProxy* Proxy = *it;
			Proxy->UpdateResources(CommandList);
		}

// ----------------------------------------------------------------------------------

#if WITH_EDITOR

		int32 NumResolvedCaptureEvents = 0;
		for (ReflectionCaptureEvent& Event : ReflectionCaptureEvents)
		{
			if (Renderer::Get()->GetFence()->IsFenceComplete(Event.CaptureFenceValue))
			{
				NumResolvedCaptureEvents++;
				ReleaseSceneRenderer(Event.CaptureSceneRenderer);


			}
		}
		if (NumResolvedCaptureEvents > 0)
		{
			ReflectionCaptureEvents.erase(ReflectionCaptureEvents.begin(), ReflectionCaptureEvents.begin() + NumResolvedCaptureEvents);
		}

		std::set<ReflectionCaptureComponent*>& ReflectionCaptures = ReflectionCaptureComponent::GetReflectionCapturesToUpdate();
		for (auto It = ReflectionCaptures.begin(); It != ReflectionCaptures.end();)
		{
			ReflectionCaptureComponent* CaptureComponent = *It;
			drn_check(CaptureComponent);

			if (CaptureComponent->GetWorld() == GetWorld())
			{
				Renderer::Get()->MarkFrameForCapture();

				It = ReflectionCaptures.erase(It);
				CaptureComponent->ClearNeedRecapture();

				for (int32 i = 0; i < 6; i++)
				{
					ReflectionCaptureEvent Event;
					Event.CaptureFenceValue = Renderer::Get()->GetFence()->GetCurrentFence();
					Event.FaceIndex = i;

					// TODO: make renderer capture only single frame. right now it captures num back buffers
					Event.CaptureSceneRenderer = AllocateSceneRenderer();
					Event.CaptureSceneRenderer->ResizeViewDeferred(IntPoint(REFLECTION_CAPTURE_SIZE));

					// TODO: add flag to scene renderer
					GetWorld()->SetGameMode(true);

					CameraActor* ViewportCamera = GetWorld()->GetViewportCamera();
					ViewportCamera->GetCameraComponent()->m_FOV = 45.0f;

					// TODO: add alternate view for scene
					Quat CameraRotation = Quat::Identity;
					CameraRotation = CameraRotation * Quat(Vector::UpVector, XM_PIDIV2);
					ViewportCamera->SetActorRotation( CameraRotation );

					Vector CameraPosition = CaptureComponent->GetWorldLocation();
					ViewportCamera->SetActorLocation( CameraPosition );

					RenderResourceCreateInfo CreateInfo(nullptr, nullptr, ClearValueBinding::BlackZeroAlpha, "ReflectionCaptureBakeTarget");
					Event.Target = RenderTexture2D::Create(Renderer::Get()->GetCommandList_Temp(), REFLECTION_CAPTURE_SIZE, REFLECTION_CAPTURE_SIZE, GBUFFER_COLOR_DEFERRED_FORMAT, 1, 1, true,
						(ETextureCreateFlags)(ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource), CreateInfo);

					Event.CaptureSceneRenderer->CopyRenderBuffer(Event.Target, ERenderBufferCopySource::FinalColorPretonemap);
					ReflectionCaptureEvents.push_back(Event);
				}
			}

			else
			{
				It++;
			}
		}
#endif
	}

	void Scene::RegisterPrimitiveProxy( PrimitiveSceneProxy* InPrimitiveSceneProxy )
	{
		m_PendingProxies.push_back(InPrimitiveSceneProxy);
	}

	void Scene::RegisterLightProxy( LightSceneProxy* InLightProxy )
	{
		m_PendingLightProxies.push_back(InLightProxy);
	}

	void Scene::RegisterSkyLightProxy( SkyLightSceneProxy* InLightProxy )
	{
		m_PendingSkyLightProxies.push_back(InLightProxy);
	}

	void Scene::RegisterPostProcessProxy( class PostProcessSceneProxy* InProxy )
	{
		m_PendingPostProcessProxies.insert(InProxy);
	}

	void Scene::UnRegisterPostProcessProxy( class PostProcessSceneProxy* InProxy )
	{
		m_PendingPostProcessProxies.erase(InProxy);
		m_PostProcessProxies.erase(InProxy);
	}

	void Scene::RegisterDecalProxy( class DecalSceneProxy* InProxy )
	{
		m_PendingDecalProxies.insert(InProxy);
	}

	void Scene::UnRegisterDecalProxy( class DecalSceneProxy* InProxy )
	{
		if (InProxy)
		{
			m_PendingDecalProxies.erase(InProxy);
			m_DecalProxies.erase(InProxy);

			InProxy->Release();
		}
	}

}