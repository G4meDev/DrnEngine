#pragma once

namespace Drn
{
	class ImGuiLayer;

	class D3D12DescriptorHeap;

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init(ID3D12Resource* MainViewportOutputBuffer);
		virtual void Tick(float DeltaTime);

		void AttachLayer(ImGuiLayer* InLayer);

		static ImGuiRenderer* Get();

	protected:
		virtual void BeginDraw();
		virtual void Draw();
		virtual void EndDraw();
		virtual void PostExecuteCommands();

		std::unique_ptr<D3D12DescriptorHeap> SrvHeap;
		
		std::unique_ptr<D3D12DescriptorHeap> MainViewportOutputHeap;

		friend class D3D12Viewport;

	private:
		static std::unique_ptr<ImGuiRenderer> SingletonInstance;
		LinkedList<ImGuiLayer> Layers;
	};
}