#include "DrnPCH.h"
#include "PreviewWorld.h"

namespace Drn
{
	PreviewWorld::PreviewWorld()
		: m_World(nullptr)
		, m_SceneRenderer(nullptr)
	{
		m_World = WorldManager::Get()->AllocateWorld();
		m_World->SetTransient(true);
		m_World->SetEditorWorld();

		DirectionalLight = m_World->SpawnActor<DirectionalLightActor>();
		SkyLight = m_World->SpawnActor<SkyLightActor>();

		m_SceneRenderer = GetScene()->AllocateSceneRenderer();
	}

	PreviewWorld::~PreviewWorld()
	{
		if (GetScene() && m_SceneRenderer)
		{
			GetScene()->ReleaseSceneRenderer(m_SceneRenderer);
		}

		if (m_World)
		{
			WorldManager::Get()->ReleaseWorld(m_World);
		}
	}

	Scene* PreviewWorld::GetScene()
	{
		return (m_World && !m_World->IsPendingDestroy()) ? m_World->GetScene() : nullptr;
	}

	SceneRenderer* PreviewWorld::GetSceneRenderer()
	{
		return m_SceneRenderer;
	}

        }  // namespace Drn