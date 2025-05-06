#pragma once

#include "ForwardTypes.h"

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

		void Render( ID3D12GraphicsCommandList2* CommandList );

		ID3D12Resource* GetViewResource();

		void ResizeView(const IntPoint& InSize);

		void SetRenderingEnabled(bool Enabled);

		const IntPoint& GetViewportSize() const { return m_RenderSize; }

		CameraActor* m_CameraActor;

	protected:

		inline void Release() { delete this; }

		Scene* m_Scene;

		Microsoft::WRL::ComPtr<ID3D12Device> m_Device;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_ColorTarget;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthTarget;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		IntPoint m_RenderSize;

		bool m_RenderingEnabled;

		friend class Scene;
		friend class Renderer;
		friend class DebugViewPhysics;
		friend class LineBatchSceneProxy;
		friend class StaticMeshSceneProxy;

	private:

		void Init(ID3D12GraphicsCommandList2* CommandList);

		void BeginRender(ID3D12GraphicsCommandList2* CommandList);
		void RenderBasePass(ID3D12GraphicsCommandList2* CommandList);
	};
}