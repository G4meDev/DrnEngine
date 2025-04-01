#include "DrnPCH.h"
#include "WorldDetailPanel.h"

#if WITH_EDITOR

#include "Editor/WorldDetailPanel/WorldDetailPanelGuiLayer.h"

namespace Drn
{
	std::unique_ptr<WorldDetailPanel> WorldDetailPanel::SingletonInstance;

	WorldDetailPanel::WorldDetailPanel()
	{

	}

	WorldDetailPanel::~WorldDetailPanel()
	{

	}

	void WorldDetailPanel::Init()
	{
		DetailLayer = std::make_unique<WorldDetailPanelGuiLayer>();
		DetailLayer->Attach();
	}

	void WorldDetailPanel::Tick(float DeltaTime)
	{

	}

	void WorldDetailPanel::Shutdown() 
	{
		SingletonInstance->DetailLayer->DeAttach();
		SingletonInstance->DetailLayer.reset();
	}

	WorldDetailPanel* WorldDetailPanel::Get()
	{
		if (!SingletonInstance)
		{
			SingletonInstance = std::make_unique<WorldDetailPanel>();
		}

		return SingletonInstance.get();
	}
}

#endif