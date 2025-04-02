#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class WorldOutlinerPanel
	{
	public:
		WorldOutlinerPanel(World* InWorld);
		~WorldOutlinerPanel();

		void Draw(float DeltaTime);

	protected:

		void DrawMenu(float DeltaTime);

		const Actor* SelectedActor;

		World* m_World;

		bool m_ShowTransient;

	private:

	};
}

#endif