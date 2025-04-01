#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class WorldDetailPanelGuiLayer : public ImGuiLayer
	{
	public:

		WorldDetailPanelGuiLayer();
		virtual ~WorldDetailPanelGuiLayer();

		virtual void Draw(float DeltaTime) override;

	protected:

	protected:
	};
}

#endif