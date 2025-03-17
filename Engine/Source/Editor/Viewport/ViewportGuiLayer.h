#pragma once

#if WITH_EDITOR
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class D3D12DescriptorHeap;

	class ViewportGuiLayer : public ImGuiLayer
	{
	public:
		ViewportGuiLayer();

		virtual void Draw() override;

	private:

		void OnViewportSizeChanged(const IntPoint& OldSize, const IntPoint& NewSize);

		IntPoint ViewportImageSize = IntPoint(800, 600);

		std::unique_ptr<D3D12DescriptorHeap> ViewportHeap;

		friend class Viewport;
	};
}

#endif