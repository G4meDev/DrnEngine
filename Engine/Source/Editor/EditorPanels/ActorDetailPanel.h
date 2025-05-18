#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Core/Delegate.h"

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnSelectedNewComponentDelegate, Component* );
	DECLARE_DELEGATE_RetVal( Component*, GetSelectedComponentDelegate);

	class ActorDetailPanel
	{
	public:
		ActorDetailPanel();
		~ActorDetailPanel();

		void Draw(float DeltaTime);


		OnSelectedNewComponentDelegate OnSelectedNewComponent;
		GetSelectedComponentDelegate GetSelectedComponentDel;

	protected:

		void DrawSceneComponents(float DeltaTime);
		void DrawNextSceneComponent(SceneComponent* Comp);
		
		void DrawDetails(float DeltaTime);

		// for caching not actual state
		Component* m_SelectedComponent;
		Actor* m_SelectedActor;

		char name[32] = "";

	private:
	};
}

#endif