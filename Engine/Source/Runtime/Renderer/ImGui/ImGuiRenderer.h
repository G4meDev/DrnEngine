#pragma once

#if WITH_EDITOR
namespace Drn
{
	class ImGuiLayer;

	class D3D12DescriptorHeap;

	class ImGuiRenderer
	{
	public:
		ImGuiRenderer();
		virtual ~ImGuiRenderer();

		virtual void Init();
		virtual void Tick(float DeltaTime);

		void AttachLayer(ImGuiLayer* InLayer);

		static ImGuiRenderer* Get();

		inline D3D12DescriptorHeap* GetSrvHeap() { return SrvHeap.get(); }

	protected:
		virtual void BeginDraw();
		virtual void Draw();
		virtual void EndDraw();
		virtual void PostExecuteCommands();

		std::unique_ptr<D3D12DescriptorHeap> SrvHeap;
		
		friend class Renderer;

	private:
		static std::unique_ptr<ImGuiRenderer> SingletonInstance;
		LinkedList<ImGuiLayer> Layers;
	};
}
#endif