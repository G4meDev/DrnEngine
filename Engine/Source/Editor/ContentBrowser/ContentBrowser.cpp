#include "DrnPCH.h"
#include "ContentBrowser.h"

#if WITH_EDITOR
#include "ContentBrowserGuiLayer.h"

namespace Drn
{
	std::unique_ptr<Drn::ContentBrowser> ContentBrowser::SingletonInstance;

	ContentBrowser::ContentBrowser()
	{
		
	}

	ContentBrowser::~ContentBrowser()
	{
		
	}

	void ContentBrowser::Init()
	{
		ContentBrowserLayer = std::make_unique<ContentBrowserGuiLayer>();
		ContentBrowserLayer->Attach();
	}

	void ContentBrowser::Tick( float DeltaTime )
	{
		
	}

	void ContentBrowser::Shutdown()
	{
		ContentBrowserLayer->DeAttach();
		ContentBrowserLayer.reset();
	}

	ContentBrowser* ContentBrowser::Get()
	{
		if ( !SingletonInstance.get() )
		{
			SingletonInstance = std::make_unique<ContentBrowser>();
		}

		return SingletonInstance.get();
	}
}

#endif