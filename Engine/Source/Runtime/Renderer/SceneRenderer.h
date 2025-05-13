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

		inline void SetName( const std::string& Name ) { m_Name = Name; }

		Guid GetGuidAtScreenPosition(const IntPoint& ScreenPosition);

		CameraActor* m_CameraActor;

	protected:

		inline void Release() { delete this; }

		Scene* m_Scene;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_ColorTarget = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_GuidTarget = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthTarget = nullptr;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		IntPoint m_RenderSize;

		bool m_RenderingEnabled;

		std::string m_Name;

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