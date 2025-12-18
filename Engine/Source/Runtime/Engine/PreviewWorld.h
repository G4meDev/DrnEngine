#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class PreviewWorld : public RefCountedObject
	{
	public:
		PreviewWorld();
		virtual ~PreviewWorld();

		class DirectionalLightActor* DirectionalLight;
		class SkyLightActor* SkyLight;
		class PostProcessVolume* PostProccessVolume;

		class Scene* GetScene();
		class SceneRenderer* GetSceneRenderer();

		class World* GetWorld() { return m_World; }

	protected:
		class World* m_World;
		class SceneRenderer* m_SceneRenderer;
	};
}