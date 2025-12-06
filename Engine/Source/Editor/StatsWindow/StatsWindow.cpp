#include "DrnPCH.h"
#include "StatsWindow.h"

#if WITH_EDITOR
#include "StatsWindowGuiLayer.h"

namespace Drn
{
	std::unique_ptr<Drn::StatsWindow> StatsWindow::SingletonInstance;

	StatsWindow::StatsWindow()
	{
		
	}

	StatsWindow::~StatsWindow()
	{
		
	}

	void StatsWindow::Init()
	{
		StatsWindowLayer = std::make_unique<StatsWindowGuiLayer>();
		StatsWindowLayer->Attach();
	}

	void StatsWindow::Tick( float DeltaTime )
	{
		
	}

	void StatsWindow::Shutdown()
	{
		StatsWindowLayer->DeAttach();
		StatsWindowLayer.reset();
	}

	StatsWindow* StatsWindow::Get()
	{
		if ( !SingletonInstance.get() )
		{
			SingletonInstance = std::make_unique<StatsWindow>();
		}

		return SingletonInstance.get();
	}
}
#endif