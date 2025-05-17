#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Core/Delegate.h"

LOG_DECLARE_CATEGORY( LogWorldOutliner );

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnSelectedNewCompponentDelegate, Component* );
	DECLARE_DELEGATE_RetVal( Component*, GetSelectedComponentDelegate )

	class WorldOutlinerPanel
	{
	public:
		WorldOutlinerPanel(World* InWorld);
		~WorldOutlinerPanel();

		void Draw(float DeltaTime);

		OnSelectedNewCompponentDelegate OnSelectedNewComponent;
		GetSelectedComponentDelegate GetSelectedComponentDel;

	protected:

		void DrawMenu(float DeltaTime);
		void DeleteActor(Actor* actor);

		World* m_World;

		bool m_ShowTransient;

	private:

	};
}

#endif