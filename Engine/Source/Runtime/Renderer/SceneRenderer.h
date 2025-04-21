#pragma once

#include "ForwardTypes.h"
#include "DebugViewPhysics.h"

LOG_DECLARE_CATEGORY(LogSceneRenderer);

namespace Drn
{
	enum class EDebugViewFlags : uint32
	{
		EShowCollision = 1
	};

	class SceneRenderer
	{
	public:

		SceneRenderer(Scene* InScene);
		virtual ~SceneRenderer();

		inline Scene* GetScene() { return m_Scene; }

		void Render(dx12lib::CommandList* CommandList);

		ID3D12Resource* GetViewResource();

		void ResizeView(const IntPoint& InSize);

		void SetRenderingEnabled(bool Enabled);

		CameraActor* m_CameraActor;

	protected:

		inline void Release() { delete this; }

		Scene* m_Scene;

		dx12lib::Device* m_Device;

		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature       = nullptr;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject = nullptr;

		std::shared_ptr<dx12lib::Texture> m_DepthTexture = nullptr;

		dx12lib::RenderTarget m_RenderTarget;

		float m_fieldOfView = 45.0f;

		bool m_RenderingEnabled;

		friend class Scene;
		friend class Renderer;
		friend class DebugViewPhysics;
		friend class LineBatchSceneProxy;

	private:

		void Init(dx12lib::CommandList* CommandList);

		void BeginRender(dx12lib::CommandList* CommandList);
		void RenderBasePass(dx12lib::CommandList* CommandList);

#if WITH_EDITOR
		DebugViewPhysics m_DebugViewPhysic;
#endif
	};
}