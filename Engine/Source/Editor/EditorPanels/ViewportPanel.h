#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include <Editor/Misc/GizmoState.h>
#include "Runtime/Core/Delegate.h"

LOG_DECLARE_CATEGORY(LogViewportPanel);

namespace Drn
{
	DECLARE_DELEGATE( OnOpenContextMenuDelegate );

	DECLARE_MULTICAST_DELEGATE_OneParam( OnSelectedNewComponentDelegate, Component* );
	DECLARE_DELEGATE_RetVal( Component*, GetSelectedComponentDelegate );

	class ViewportPanel
	{
	public:
		ViewportPanel(Scene* InScene);
		~ViewportPanel();

		void Draw(float DeltaTime);

		void SetRenderingEnabled(bool Enabled);

		OnOpenContextMenuDelegate OnOpenContextMenu;

		OnSelectedNewComponentDelegate OnSelectedNewComponent;
		GetSelectedComponentDelegate GetSelectedComponentDel;

	protected:

		void OnViewportSizeChanged( const IntPoint& NewSize );
		void DrawHeader();
		void HandleInputs();

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

		IntPoint CachedSize = IntPoint( 1920, 1080 );

		CameraActor* m_ViewportCamera;
		StaticMeshActor* m_GridActor;

		ViewportCameraInputHandler CameraInputHandler;
		float CameraMovementSpeed = 0.01f;
		float CameraRotationSpeed = 0.01f;

		GizmoState m_GizmoState;

		World* m_World;
		Scene* m_Scene;
		SceneRenderer* m_SceneRenderer;

	private:
	};
}

#endif