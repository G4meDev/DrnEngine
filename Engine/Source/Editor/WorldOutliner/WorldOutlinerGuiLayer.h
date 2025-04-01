#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY(LogWorldOutliner);

namespace Drn
{
	class WorldOutlinerGuiLayer : public ImGuiLayer
	{
	public:
		WorldOutlinerGuiLayer();
		virtual ~WorldOutlinerGuiLayer();

		virtual void Draw(float DeltaTime);

	protected:

		void DrawActorList();

		World* GetMainWorld();

		const Actor* SelectedActor;

	protected:

	};
}

#endif