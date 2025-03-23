#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class ContentBrowserGuiLayer;

	class ContentBrowser
	{
	public:
		ContentBrowser();
		~ContentBrowser();

		void Init();
		void Tick( float DeltaTime );
		void Shutdown();

		static ContentBrowser* Get();

	protected:
		std::unique_ptr<ContentBrowserGuiLayer> ContentBrowserLayer;

	private:
		static std::unique_ptr<ContentBrowser> SingletonInstance;
		friend class OutputLogGuiLayer;
	};
}

#endif