#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class WorldOutlinerGuiLayer;

	class WorldOutliner
	{
	public:

		WorldOutliner();
		~WorldOutliner();

		void Init();
		void Tick(float DeltaTime);
		void Shutdown();

		static WorldOutliner* Get();

	protected:

		std::unique_ptr<WorldOutlinerGuiLayer> WorldOutlinerLayer;

	private:
		
		static std::unique_ptr<WorldOutliner> SingletonInstance;

		friend class ViewportGuiLayer;

	};
}

#endif