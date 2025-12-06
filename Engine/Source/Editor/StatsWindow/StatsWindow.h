#pragma once

#if WITH_EDITOR
#include "ForwardTypes.h"

namespace Drn
{
	class StatsWindowGuiLayer;

	class StatsWindow
	{
	public:
		StatsWindow();
		~StatsWindow();

		void Init();
		void Tick( float DeltaTime );
		void Shutdown();

		static StatsWindow* Get();

	protected:
		std::unique_ptr<StatsWindowGuiLayer> StatsWindowLayer;

	private:
		static std::unique_ptr<StatsWindow> SingletonInstance;

		friend class StatsWindowGuiLayer;
	};
}

#endif