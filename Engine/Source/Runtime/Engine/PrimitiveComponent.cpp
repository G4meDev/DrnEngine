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
		, m_EditorPrimitive(false)
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

		//m_BodyInstance.AddForce(Vector::ForwardVector * 10, false);
	}

	void PrimitiveComponent::SendPhysicsTransform()
	{
		m_BodyInstance.SetBodyTransform(GetWorldTransform());
		m_BodyInstance.UpdateBodyScale(GetWorldTransform().GetScale());
	}

	void PrimitiveComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

	}

	void PrimitiveComponent::UnRegisterComponent()
	{
		SceneComponent::UnRegisterComponent();

	}

	void PrimitiveComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform( SkipPhysic );
		
		if (!SkipPhysic)
		{
			SendPhysicsTransform();
		}
	}

	void PrimitiveComponent::SetSelectable( bool Selectable )
	{
		m_Selectable = Selectable;
	}

#if WITH_EDITOR
	void PrimitiveComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		m_BodyInstance.DrawDetailPanel(DeltaTime);
	}

	void PrimitiveComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		SceneComponent::SetSelectedInEditor( SelectedInEditor );


	}

#endif

}