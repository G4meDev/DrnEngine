#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogSceneRenderer);

namespace Drn
{
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

		Scene* m_Scene;

		dx12lib::Device* m_Device;

		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature       = nullptr;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject = nullptr;

		std::shared_ptr<dx12lib::Texture> m_DepthTexture = nullptr;

		dx12lib::RenderTarget m_RenderTarget;

		float m_fieldOfView = 45.0f;

		bool m_RenderingEnabled;

		friend class Renderer;

	private:

		void Init(dx12lib::CommandList* CommandList);

		void BeginRender(dx12lib::CommandList* CommandList);
		void RenderBasePass(dx12lib::CommandList* CommandList);
	};
}