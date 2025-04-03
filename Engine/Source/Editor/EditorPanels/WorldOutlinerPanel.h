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

		void DeleteActor(Actor* actor);

		Actor* m_SelectedActor;

		World* m_World;

		bool m_ShowTransient;

	private:

		void OnRemovedActorFromWorld(const Actor* RemovedActor);

	};
}

#endif