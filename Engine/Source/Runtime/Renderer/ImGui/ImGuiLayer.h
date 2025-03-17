#pragma once

namespace Drn
{
	class ImGuiLayer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();

		virtual void Draw() = 0;

		void Attach();
		void DeAttach();
	};
}