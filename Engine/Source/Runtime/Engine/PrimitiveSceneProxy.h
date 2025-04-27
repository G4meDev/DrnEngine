#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class PrimitiveSceneProxy
	{
	public:
		PrimitiveSceneProxy( const PrimitiveComponent* InComponent);
		virtual ~PrimitiveSceneProxy();

		virtual void Destroy();

	protected:

		virtual void RenderMainPass(dx12lib::CommandList* CommandList, SceneRenderer* Renderer) const = 0;

		virtual void InitResources(dx12lib::CommandList* CommandList) = 0;
		virtual void UpdateResources(dx12lib::CommandList* CommandList) = 0;
		virtual PrimitiveComponent* GetPrimitive() = 0;

	private:

		friend class Scene;
		friend class Renderer;
		friend class SceneRenderer;
	};
}