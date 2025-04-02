#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class ActorDetailPanel
	{
	public:
		ActorDetailPanel();
		~ActorDetailPanel();

		void Draw(float DeltaTime);

		void SetSelectedActor(Actor* SelectedActor);

	protected:

		void DrawSceneComponents(float DeltaTime);
		void DrawNextSceneComponent(SceneComponent* Comp);
		
		void DrawDetails(float DeltaTime);

		//void DrawComponents(float DeltaTime);

	private:

		Actor* m_SelectedActor;
		Component* m_SelectedComponent;
	};
}

#endif