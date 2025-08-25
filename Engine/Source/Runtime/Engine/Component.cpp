#include "DrnPCH.h"
#include "Component.h"

namespace Drn
{
	Component::Component()
		: m_Guid(Guid::NewGuid())
		, m_Registered(false)
		, m_PendingKill(false)
	{
		
	}

	Component::~Component()
	{
		
	}

	void Component::Tick( float DeltaTime )
	{
		
	}

	Actor* Component::GetOwningActor() const
	{
		return Owner;
	}

	void Component::SetOwningActor(Actor* InActor)
	{
		Owner = InActor;
	}

	bool Component::IsActive() const
	{
		return bActive;
	}

	void Component::SetActive(bool Active)
	{
		bActive = Active;
	}

	void Component::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			//Ar >> ComponentLabel;
		}

		else
		{
			//Ar << ComponentLabel;
		}
	}

#if WITH_EDITOR
	std::string Component::GetComponentLabel() const
	{
		return ComponentLabel;
	}

	void Component::SetComponentLabel( const std::string& InLabel )
	{
		ComponentLabel = InLabel;
	}

	void Component::SetSelectedInEditor( bool SelectedInEditor )
	{
		m_SelectedInEditor = SelectedInEditor;
	}

	void Component::MarkPendingKill()
	{
		m_PendingKill = true;
	}

#else
	std::string Component::GetComponentLabel() const
	{
		return "InvalidName";
	}

	void Component::SetComponentLabel( const std::string& InLabel )
	{
	}
#endif

	void Component::RegisterComponent( World* InOwningWorld )
	{
		m_OwningWorld = InOwningWorld;
		m_Registered = true;
	}

	void Component::UnRegisterComponent()
	{
		m_OwningWorld = nullptr;
	}

	void Component::DestroyComponent()
	{
		if (!m_PendingKill)
		{
			if (IsRegistered())
			{
				UnRegisterComponent();
			}

			Owner->RemoveOwnedComponent(this);
			MarkPendingKill();
		}
	}

}