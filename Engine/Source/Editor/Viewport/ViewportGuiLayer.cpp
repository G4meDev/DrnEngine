#include "DrnPCH.h"
#include "ViewportGuiLayer.h"

#if WITH_EDITOR

#include "imgui.h"
//#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	void ViewportGuiLayer::Draw()
	{
		ImGui::Begin("Viewport", (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar);

		const ImVec2 AvaliableSize = ImGui::GetContentRegionAvail();
		const IntPoint ImageSize = IntPoint((int32)AvaliableSize.x, (int32)AvaliableSize.y);

		if (ViewportImageSize != ImageSize)
		{
			OnViewportSizeChanged(ViewportImageSize, ImageSize);
			ViewportImageSize = ImageSize;
		}

		//ImGui::Image((void*)(size_t)(Renderer::Gbuffer->Attachments[7]->TextureID), AvaliableSize,
		//	ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
	}

	void ViewportGuiLayer::OnViewportSizeChanged(const IntPoint& OldSize, const IntPoint& NewSize)
	{
		//Viewport::Get()->OnViewportSizeChanged(OldSize, NewSize);
	}

}
#endif