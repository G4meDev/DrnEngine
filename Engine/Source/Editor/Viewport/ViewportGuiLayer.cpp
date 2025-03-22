#include "DrnPCH.h"
#include "ViewportGuiLayer.h"

#if 0

#include "imgui.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/D3D12Scene.h"

#include "Editor/Editor.h"

namespace Drn
{
	ViewportGuiLayer::ViewportGuiLayer(D3D12Scene* InScene)
	{
		Scene = InScene;

		ViewportHeap = std::make_unique<D3D12DescriptorHeap>(ImGuiRenderer::Get()->GetSrvHeap());

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Renderer::Get()->Adapter->GetD3DDevice()->CreateShaderResourceView(Scene->GetOutputBuffer(), &descSRV, ViewportHeap->GetCpuHandle());
	}

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

		ImGui::Image(ImTextureID(ViewportHeap->GetGpuHandle().ptr), ImVec2(Renderer::Get()->GetMainScene()->GetSize().X, Renderer::Get()->GetMainScene()->GetSize().Y));

		ImGui::End();
	}

	void ViewportGuiLayer::OnViewportSizeChanged(const IntPoint& OldSize, const IntPoint& NewSize)
	{
		LOG(LogEditor, Info, "Resize viewport from %s to %s.", OldSize.ToString().c_str(), NewSize.ToString().c_str());

		Scene->Resize(NewSize);
	}

}
#endif