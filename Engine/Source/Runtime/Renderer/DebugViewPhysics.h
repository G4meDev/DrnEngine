#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class DebugViewPhysics
	{
	public:
		DebugViewPhysics();

		void Init(SceneRenderer* InSceneRenderer, dx12lib::CommandList* CommandList);
		void Shutdown();

		void RenderCollisions(dx12lib::CommandList* CommandList);
		void RenderPhysxDebug(dx12lib::CommandList* CommandList);

	private:
		
		std::shared_ptr<dx12lib::PipelineStateObject> m_CollisionPSO = nullptr;

		std::vector<StaticMeshVertexBuffer> CollisionVertexData;
		std::shared_ptr<dx12lib::VertexBuffer> CollisionVertexBuffer;

		std::vector<uint32> CollisionIndexData;
		std::shared_ptr<dx12lib::IndexBuffer> CollisionIndexBuffer;

		SceneRenderer* m_SceneRenderer;
	};
}