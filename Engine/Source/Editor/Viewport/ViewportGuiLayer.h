#pragma once

#if 0
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class D3D12DescriptorHeap;
	class D3D12Scene;

	class ViewportGuiLayer : public ImGuiLayer
	{
	public:
		ViewportGuiLayer(D3D12Scene* InScene);

		virtual void Draw() override;

	private:

		void OnViewportSizeChanged(const IntPoint& OldSize, const IntPoint& NewSize);

		IntPoint ViewportImageSize = IntPoint(800, 600);

		std::unique_ptr<D3D12DescriptorHeap> ViewportHeap;

		D3D12Scene* Scene;

		friend class Viewport;
	};
}

#endif