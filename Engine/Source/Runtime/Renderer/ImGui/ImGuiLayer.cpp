#include "DrnPCH.h"
#include "ImGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	ImGuiLayer::ImGuiLayer()
		: m_Open(true)
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{
		if (OnLayerClose.IsBound())
		{
			OnLayerClose.Execute();
		}
	}

	void ImGuiLayer::Draw( float DeltaTime )
	{

	}

	void ImGuiLayer::Attach()
	{
		ImGuiRenderer::Get()->AttachLayer(this);
	}

	void ImGuiLayer::DeAttach()
	{
		ImGuiRenderer::Get()->DetachLayer(this);
	}


}

#endif