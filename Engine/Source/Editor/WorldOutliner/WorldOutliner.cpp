#include "DrnPCH.h"
#include "WorldOutliner.h"

#if WITH_EDITOR

#include "WorldOutlinerGuiLayer.h"

namespace Drn
{
	std::unique_ptr<Drn::WorldOutliner> WorldOutliner::SingletonInstance;

	WorldOutliner::WorldOutliner()
	{
		
	}

	WorldOutliner::~WorldOutliner()
	{
		
	}

	void WorldOutliner::Init()
	{
		WorldOutlinerLayer = std::make_unique<WorldOutlinerGuiLayer>();
		WorldOutlinerLayer->Attach();
	}

	void WorldOutliner::Shutdown()
	{
		SingletonInstance->WorldOutlinerLayer->DeAttach();
		SingletonInstance->WorldOutlinerLayer.reset();
	}

	void WorldOutliner::Tick( float DeltaTime )
	{

	}


	WorldOutliner* WorldOutliner::Get()
	{
		if (!SingletonInstance)
		{
			SingletonInstance = std::make_unique<Drn::WorldOutliner>();
		}

		return SingletonInstance.get();
	}


}

#endif