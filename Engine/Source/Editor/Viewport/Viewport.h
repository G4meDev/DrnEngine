#pragma once

#if WITH_EDITOR

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
		void Shutdown();

		static Viewport* Get();
		void OnViewportSizeChanged(const IntPoint& NewSize);

		inline bool IsVisible() { return m_Visible; }

	protected:
		std::unique_ptr<ViewportGuiLayer> ViewportLayer;

		bool m_Visible;

	private:
		static std::unique_ptr<Viewport> SingletonInstance;

		friend class ViewportGuiLayer;
	};
}

#endif