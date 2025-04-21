#include "DrnPCH.h"
#include "PrimitiveComponent.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	PrimitiveComponent::PrimitiveComponent()
		: SceneComponent()
		, m_RenderStateDirty(true)
	{
		
	}

	PrimitiveComponent::~PrimitiveComponent()
	{
		
	}

	void PrimitiveComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			m_BodyInstance.Serialize(Ar);

		}

		else
		{
			m_BodyInstance.Serialize(Ar);

		}
	}

	void PrimitiveComponent::Tick( float DeltaTime )
	{
		SceneComponent::Tick(DeltaTime);
	}

	void PrimitiveComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);


	}

	void PrimitiveComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();


	}


#if WITH_EDITOR
	void PrimitiveComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		m_BodyInstance.DrawDetailPanel(DeltaTime);
	}
#endif

}