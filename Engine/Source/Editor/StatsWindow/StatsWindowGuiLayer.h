#pragma once

#if WITH_EDITOR
#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class StatsWindowGuiLayer: public ImGuiLayer
	{
	public:
		StatsWindowGuiLayer();
		~StatsWindowGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	private:
	};
}

#endif