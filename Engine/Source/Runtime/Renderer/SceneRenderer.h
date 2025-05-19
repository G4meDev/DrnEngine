#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/Resource.h"
#include "Runtime/Core/Delegate.h"

LOG_DECLARE_CATEGORY(LogSceneRenderer);

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnPickedComponentDelegate, Component*);

	enum class EDebugViewFlags : uint32
	{
		EShowCollision = 1
	};

	struct MousePickEvent
	{
		MousePickEvent( const IntPoint& InScreenPos )
			: ScreenPos(InScreenPos)
		{
		}

		MousePickEvent() {}

		~MousePickEvent()
		{
			if (ReadbackBuffer)
			{
				ReadbackBuffer->ReleaseBufferedResource();
			}
		}

		// TODO: Add event to trigger
		IntPoint ScreenPos;
		bool Initalized = false;
		Resource* ReadbackBuffer = nullptr;
		uint64 FenceValue = 0;
	};

	class SceneRenderer
	{
	public:

		SceneRenderer(Scene* InScene);
		virtual ~SceneRenderer();

		inline Scene* GetScene() { return m_Scene; }

		void Render( ID3D12GraphicsCommandList2* CommandList );

		ID3D12Resource* GetViewResource();

		void ResizeView( const IntPoint& InSize );

		void SetRenderingEnabled(bool Enabled);

		const IntPoint& GetViewportSize() const { return m_RenderSize; }

		inline void SetName( const std::string& Name ) { m_Name = Name; }

#if WITH_EDITOR
		OnPickedComponentDelegate OnPickedComponent;

		void QueueMousePickEvent( const IntPoint& ScreenPosition );
		Microsoft::WRL::ComPtr<ID3D12Fence> m_MousePickFence;
		uint64 m_FenceValue = 0;
#endif

		CameraActor* m_CameraActor;

	protected:

		inline void Release() { delete this; }

		Scene* m_Scene;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap = nullptr;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_ColorTarget = nullptr;
		//Microsoft::WRL::ComPtr<ID3D12Resource> m_GuidTarget = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthTarget = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_EditorColorTarget = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_EditorDepthTarget = nullptr;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_EditorSelectionDepthStencilTarget = nullptr;

		D3D12_CPU_DESCRIPTOR_HANDLE m_EditorColorCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_EditorColorGpuHandle;

		D3D12_CPU_DESCRIPTOR_HANDLE m_EditorSelectionDepthStencilCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_EditorSelectionDepthStencilGpuHandle;

		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		IntPoint m_RenderSize;

		bool m_RenderingEnabled;

		std::string m_Name;

#if WITH_EDITOR
		void RenderHitProxyPass(ID3D12GraphicsCommandList2* CommandList);

		void ProccessMousePickQueue(ID3D12GraphicsCommandList2* CommandList);
		void KickstartMousePickEvent( MousePickEvent& Event );
		std::vector<MousePickEvent> m_MousePickQueue;

		std::shared_ptr<class HitProxyRenderBuffer> m_HitProxyRenderBuffer;
#endif

		friend class Scene;
		friend class Renderer;
		friend class DebugViewPhysics;
		friend class LineBatchSceneProxy;
		friend class StaticMeshSceneProxy;

	private:

		void Init(ID3D12GraphicsCommandList2* CommandList);

		void BeginRender(ID3D12GraphicsCommandList2* CommandList);
		void RenderBasePass(ID3D12GraphicsCommandList2* CommandList);

#if WITH_EDITOR
		void RenderEditorPrimitives(ID3D12GraphicsCommandList2* CommandList);
#endif
	};
}