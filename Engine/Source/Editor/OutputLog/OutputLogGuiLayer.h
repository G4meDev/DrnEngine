#pragma once

#if WITH_EDITOR
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

namespace Drn
{
	class OutputLogGuiLayer : public ImGuiLayer
	{
	public:
		virtual void Draw(float DeltaTime) override;

	private:
		void MakeMenuBar();
		void MakeLogsTable();

		void GetQualifiedLogsIndex(std::vector<int32>& Indices);
	};

}

#endif
