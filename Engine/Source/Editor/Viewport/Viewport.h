#pragma once

#if 0

namespace Drn
{
	class ViewportGuiLayer;

	class Viewport
	{
	public:
		Viewport();
		~Viewport();

		void Init();
		void Tick(float DeltaTime);

		static Viewport* Get();
		void OnViewportSizeChanged(const IntPoint& OldSize, const IntPoint& NewSize);

	protected:
		std::unique_ptr<ViewportGuiLayer> ViewportLayer;

	private:
		static std::unique_ptr<Viewport> SingletonInstance;

		friend class ViewportGuiLayer;
	};
}

#endif