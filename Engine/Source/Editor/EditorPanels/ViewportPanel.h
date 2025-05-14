#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include <Editor/Misc/GizmoState.h>

LOG_DECLARE_CATEGORY(LogViewportPanel);

namespace Drn
{
	class ViewportPanel
	{
	public:
		ViewportPanel(Scene* InScene);
		~ViewportPanel();

		void Draw(float DeltaTime);

		void SetRenderingEnabled(bool Enabled);

	protected:

		void OnViewportSizeChanged( const IntPoint& NewSize );
		void DrawHeader();
		void HandleInputs();

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

		IntPoint CachedSize = IntPoint( 1920, 1080 );

		CameraActor* m_ViewportCamera;

		Component* m_SelectedComponent;

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