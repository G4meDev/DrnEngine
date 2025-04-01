#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class WorldOutlinerPanel
	{
	public:
		WorldOutlinerPanel();
		~WorldOutlinerPanel();

		void Draw(float DeltaTime);

	protected:

		World* GetMainWorld();

		const Actor* SelectedActor;

	private:

	};
}

#endif