#pragma once

#if WITH_EDITOR
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include "Editor/Misc/ViewportCameraInputHandler.h"

namespace Drn
{
	class ViewportGuiLayer : public ImGuiLayer
	{
	public:
		ViewportGuiLayer();
		~ViewportGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	private:

		void ShowMenu();

		void OnViewportSizeChanged( const IntPoint& NewSize);

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

		IntPoint CachedSize = IntPoint( 1920, 1080 );

		ViewportCameraInputHandler CameraInputHandler;

		friend class Viewport;
	};
}

#endif