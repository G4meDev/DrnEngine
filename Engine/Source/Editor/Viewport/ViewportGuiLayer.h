#pragma once

#if WITH_EDITOR
    #include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{

	class ViewportGuiLayer : public ImGuiLayer
	{
	public:
		ViewportGuiLayer();

		virtual void Draw() override;

	private:

		void OnViewportSizeChanged( const IntPoint& NewSize);

		D3D12_CPU_DESCRIPTOR_HANDLE ViewCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE ViewGpuHandle;

		IntPoint CachedSize = IntPoint( 1920, 1080 );

		friend class Viewport;
	};
}

#endif