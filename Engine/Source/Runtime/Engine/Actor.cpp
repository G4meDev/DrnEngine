#include "DrnPCH.h"
#include "Actor.h"

LOG_DEFINE_CATEGORY( LogActor, "Actor" );

namespace Drn
{
	Actor::Actor()
		: Root(nullptr)
		, m_World(nullptr)
		, m_PendingKill(false)
	{

	}

	Actor::~Actor()
	{
		
	}

	void Actor::Tick(float DeltaTime)
	{
		for (auto Comp : Components)
		{
			Comp->Tick(DeltaTime);
		}

		for (auto SceneComp : Root->GetChilds())
		{
			SceneComp->Tick(DeltaTime);
		}
	}

	void Actor::GetComponentsInline( std::vector<Component*>& Comps )
	{
		Comps = Components;
		Root->GetComponentsInline(Comps);
	}

	Vector Actor::GetActorLocation() const
	{
		return Root->GetWorldLocation();
	}

	void Actor::SetActorLocation( const Vector& InLocation )
	{
		Root->SetWorldLocation(InLocation);
	}


	Quat Actor::GetActorRotation() const
	{
		return Root->GetWorldRotation();
	}

	void Actor::SetActorRotation( const Quat& InRotator )
	{
		Root->SetWorldRotation(InRotator);
	}

	//void Actor::AddRelativeRotation( const Quat& InRotator )
	//{
	//	Root->AddRelativeRotation(InRotator);
	//}

	//void Actor::AddWorldRotation( const Quat& InRotator )
	//{
	//	Root->AddWorldRotation(InRotator);
	//}

	Vector Actor::GetActorScale() const
	{
		return Root->GetWorldScale();
	}

	void Actor::SetActorScale( const Vector& InScale )
	{
		Root->SetWorldScale(InScale);
	}

	Transform Actor::GetActorTransform() const
	{
		return Root->GetWorldTransform();
	}

	void Actor::SetActorTransform( const Transform& InTransform )
	{
		Root->SetWorldTransform(InTransform);
	}

	Vector Actor::GetActorForwardVector() const
	{
		return Root ? Root->GetForwardVector() : Vector::ForwardVector;
	}

	Vector Actor::GetActorUpVector() const
	{
		return Root ? Root->GetUpVector() : Vector::UpVector;
	}

	Vector Actor::GetActorRightVector() const
	{
		return Root ? Root->GetRightVector() : Vector::RightVector;
	}

	void Actor::AttachSceneComponent( SceneComponent* InSceneComponent, SceneComponent* Target )
	{
		if (!Target)
		{
			Target = Root;
		}

		if (Target->GetOwningActor() == this)
		{
			Target->AttachSceneComponent(InSceneComponent);
			InSceneComponent->SetOwningActor(this);
		}
	}

	void Actor::AddComponent(Component* InComponent)
	{
		Components.push_back(InComponent);
		InComponent->SetOwningActor(this);
	}

	SceneComponent* Actor::GetRoot() const
	{
		return Root;
	}

	void Actor::Destroy()
	{
		if (!m_PendingKill)
		{
			m_PendingKill = true;
			OnActorKilled.Braodcast(this);
		}

	}

	bool Actor::IsMarkedPendingKill() const
	{
		return m_PendingKill;
	}

	EActorType Actor::GetActorType() {}

	void Actor::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			std::string ActorLabelStr;
			Ar >> ActorLabelStr;

#if WITH_EDITOR
			ActorLabel = ActorLabelStr;
#endif

			Root->Serialize(Ar);
		}

#if WITH_EDITOR

		else
		{
			Ar << ActorLabel;
			Root->Serialize(Ar);
		}

#endif
	}

	void Actor::GetActorEyesViewPoint( Vector& OutLocation, Quat& OutRotation ) const
	{
		OutLocation = GetActorLocation();
		OutRotation = GetActorRotation();
	}

	void Actor::CalcCamera( struct ViewInfo& OutResult )
	{
		GetActorEyesViewPoint(OutResult.Location, OutResult.Rotation);
	}

#if WITH_EDITOR
	void Actor::DrawEditorSelected()
	{
		std::vector<Component*> Comps;
		GetComponentsInline(Comps);

		for (Component* Comp : Comps)
		{
			Comp->DrawEditorSelected();
		}
	}

	std::string Actor::GetActorLabel() const
	{
		return ActorLabel;
	}

	void Actor::SetActorLabel( const std::string& InLabel )
	{
		ActorLabel = InLabel;
	}

	void Actor::SetTransient( bool Transient )
	{
		m_Transient = Transient;
	}

	void Actor::SetComponentsSelectedInEditor( bool SelectedInEditor )
	{
		std::vector<Component*> AllComponents;
		GetComponents<Component>(AllComponents, EComponentType::Component, true);

		for (Component* Comp : AllComponents)
		{
			Comp->SetSelectedInEditor(SelectedInEditor);
		}
	}
#endif

	void Actor::RegisterComponents( World* InWorld )
	{
		for (auto Comp : Components)
		{
			if(!Comp->IsRegistered())
				Comp->RegisterComponent(InWorld);
		}

		RegisterSceneComponentRecursive(Root, InWorld);
	}

	void Actor::UnRegisterComponents()
	{
		for (auto Comp : Components)
		{
			if(Comp->IsRegistered())
				Comp->UnRegisterComponent();
		}

		UnRegisterSceneComponentRecursive(Root);
	}

	void Actor::PostInitializeComponents()
	{
		
	}

	void Actor::DispatchPhysicsCollisionHit( const RigidBodyCollisionInfo& MyInfo,
		const RigidBodyCollisionInfo& OtherInfo, const CollisionImpactData& RigidCollisionData )
	{
		for ( const RigidBodyContactInfo& ContactInfo : RigidCollisionData.ContactInfos )
		{
			Root->GetWorld()->DrawDebugSphere( ContactInfo.ContactPosition, Quat::Identity, Color::Blue, 0.2f, 8, 0.05, 0 );
			Root->GetWorld()->DrawDebugLine( ContactInfo.ContactPosition, ContactInfo.ContactPosition + ContactInfo.ContactNormal * 0.5, Color::Blue, 0.05f, 0 );
		}
	}

	void Actor::RegisterSceneComponentRecursive( SceneComponent* InComponent, World* InWorld )
	{
		if (!InComponent->IsRegistered())
			InComponent->RegisterComponent(InWorld);

		for ( auto Child: InComponent->GetChilds() )
		{
			RegisterSceneComponentRecursive(Child, InWorld);
		}
	}

	void Actor::UnRegisterSceneComponentRecursive( SceneComponent* InComponent )
	{
		if (InComponent->IsRegistered())
			InComponent->UnRegisterComponent();

		for ( auto Child: InComponent->GetChilds() )
		{
			UnRegisterSceneComponentRecursive(Child);
		}
	}

	void Actor::RemoveOwnedComponent( Component* InComponent )
	{
		if (InComponent)
		{
			for ( int32 i = 0; i < Components.size(); i++ )
			{
				Component*& Comp = Components[i];
				if (Comp == InComponent)
				{
					Components.erase(Components.begin() + i);
					return;
				}
			}
		}
	}

}