#pragma once

namespace Drn
{
	class ImGuiLayer;

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init();
		virtual void Tick(float DeltaTime);

		void AttachLayer(ImGuiLayer* InLayer);

		static ImGuiRenderer* Get();

	protected:
		virtual void BeginDraw();
		virtual void Draw();
		virtual void EndDraw();

	private:
		static std::unique_ptr<ImGuiRenderer> SingletonInstance;
		LinkedList<ImGuiLayer> Layers;
	};
}