#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class WorldDetailPanelGuiLayer;

	class WorldDetailPanel
	{
	public:
		WorldDetailPanel();
		~WorldDetailPanel();

		void Init();
		void Shutdown();
		void Tick(float DeltaTime);

		static WorldDetailPanel* Get();

	protected:
		std::unique_ptr<WorldDetailPanelGuiLayer> DetailLayer;

	private:
		static std::unique_ptr<WorldDetailPanel> SingletonInstance;

		friend class WorldDetailPanelGuiLayer;
	};
}

#endif