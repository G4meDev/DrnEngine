#include "DrnPCH.h"
#include "ImGuiLayer.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	ImGuiLayer::ImGuiLayer()
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::Draw()
	{

	}

	void ImGuiLayer::Attach()
	{
		ImGuiRenderer::Get()->AttachLayer(this);
	}

	void ImGuiLayer::DeAttach()
	{

	}


}