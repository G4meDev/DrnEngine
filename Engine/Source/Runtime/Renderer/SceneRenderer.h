#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/Resource.h"
#include "Runtime/Core/Delegate.h"

LOG_DECLARE_CATEGORY(LogSceneRenderer);

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnPickedComponentDelegate, Component* );
	DECLARE_MULTICAST_DELEGATE_OneParam( OnSceneRendererResizedDelegate, const IntPoint& );
	DECLARE_MULTICAST_DELEGATE( OnSceneRendererDestroyDelegate );

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

	struct SceneRendererView
	{
		SceneRendererView()
		{
		}
		~SceneRendererView()
		{
		}

		Matrix WorldToView;
		Matrix ViewToProjection;
		Matrix WorldToProjection;
		Matrix ProjectionToView;
		Matrix ProjectionToWorld;
		Matrix LocalToCameraView;

		IntPoint Size;
		uint32 Pad_1;
		uint32 Pad_2;
		
		Vector CameraPos;
		float Pad_3;
		Vector CameraDir;
	};

	class SceneRenderer
	{
	public:

		SceneRenderer(Scene* InScene);
		virtual ~SceneRenderer();

		inline Scene* GetScene() { return m_Scene; }

		void Render();

		ID3D12Resource* GetViewResource();

		void ResizeView( const IntPoint& InSize );
		void ResizeViewDeferred( const IntPoint& InSize );
		void ResizeViewConditional();

		void SetRenderingEnabled(bool Enabled);

		const IntPoint& GetViewportSize() const { return m_CachedRenderSize; }

		inline void SetName( const std::string& Name ) { m_Name = Name; }
		inline const std::string& GetName() const { return m_Name; }

		inline const SceneRendererView& GetSceneView() { return m_SceneView; }

#if WITH_EDITOR
		OnPickedComponentDelegate OnPickedComponent;

		void QueueMousePickEvent( const IntPoint& ScreenPosition );
		Microsoft::WRL::ComPtr<ID3D12Fence> m_MousePickFence;
		uint64 m_FenceValue = 0;
#endif

		CameraActor* m_CameraActor;

		OnSceneRendererResizedDelegate OnSceneRendererResized;
		OnSceneRendererDestroyDelegate OnSceneRendererDestroy;

		tf::Taskflow m_RenderTask;

		Resource* m_BindlessViewBuffer = nullptr;

	protected:

		inline void Release() { delete this; }

		void RecalculateView();

		Scene* m_Scene;

		SceneRendererView m_SceneView;

		std::shared_ptr<class GBuffer> m_GBuffer;
		std::shared_ptr<class TonemapRenderBuffer> m_TonemapBuffer;

		IntPoint m_CachedRenderSize;
		IntPoint m_RenderSize;

		bool m_RenderingEnabled;

		std::string m_Name;


		void UpdateViewBuffer();

#if WITH_EDITOR
		void RenderHitProxyPass();

		void ProccessMousePickQueue();
		void KickstartMousePickEvent( MousePickEvent& Event );
		// event get queued from viewport/imgui renderer. right now imgui renderer is not paralleled, if remember to lock
		std::vector<MousePickEvent> m_MousePickQueue;

		std::shared_ptr<class HitProxyRenderBuffer> m_HitProxyRenderBuffer;
		std::shared_ptr<class EditorPrimitiveRenderBuffer> m_EditorPrimitiveBuffer;
		std::shared_ptr<class EditorSelectionRenderBuffer> m_EditorSelectionBuffer;
#endif

		friend class Scene;
		friend class Renderer;
		friend class DebugViewPhysics;
		friend class LineBatchSceneProxy;
		friend class StaticMeshSceneProxy;

	private:

		void Init();

		void RenderShadowDepths();
		void RenderBasePass();
		void RenderLights();
		void RenderPostProcess();
		void PostProcess_Tonemapping();

#if WITH_EDITOR
		void RenderEditorPrimitives();
		void RenderEditorSelection();
#endif

		D3D12CommandList* m_CommandList;
	};
}