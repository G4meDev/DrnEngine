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

		inline Actor* GetSelectedActor() { return m_SelectedActor; }

	protected:

		void DrawMenu(float DeltaTime);

		Actor* m_SelectedActor;

		World* m_World;

		bool m_ShowTransient;

	private:

	};
}

#endif