#include "DrnPCH.h"
#include "Actor.h"

namespace Drn
{
	Actor::Actor()
	{
		Root = std::make_unique<SceneComponent>();
		Root->SetOwningActor(this);

		m_PendingKill = false;

#if WITH_EDITOR
		Root->SetComponentLabel( "Root" );
#endif
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

	DirectX::XMVECTOR Actor::GetActorLocation()
	{
		return Root->GetWorldLocation();
	}

	void Actor::SetActorLocation( const DirectX::XMVECTOR& InLocation )
	{
		Root->SetWorldLocation(InLocation);
		//Root->MarkDirtyLocationRecursive();
	}


	DirectX::XMVECTOR Actor::GetActorRotation()
	{
		return Root->GetWorldRotation();
	}

	void Actor::SetActorRotation( const DirectX::XMVECTOR& InRotator )
	{
		Root->SetWorldRotation(InRotator);
		//Root->MarkDirtyRotationRecursive();
	}

/*
	const Vector3 Actor::GetActorScale()
	{
		return Root->GetWorldScale();
	}

	void Actor::SetActorScale(const Vector3& InScale)
	{
		Root->SetWorldScale(InScale);
		Root->MarkDirtyScaleRecursive();
	}
*/

	void Actor::AttachSceneComponent(SceneComponent* InSceneComponent, SceneComponent* Target)
	{
		if (!Target)
		{
			Target = Root.get();
		}

		if (Target->GetOwningActor() == this)
		{
			Target->AttachSceneComponent(InSceneComponent);
			InSceneComponent->SetOwningActor(this);
		}
	}

	void Actor::AddComponent(Component* InComponent)
	{
		Components.push_back(std::shared_ptr<Component>(InComponent));
		InComponent->SetOwningActor(this);
	}

	SceneComponent* Actor::GetRoot() const
	{
		return Root.get();
	}

	void Actor::Destroy()
	{
		m_PendingKill = true;
	}

	bool Actor::IsMarkedPendingKill() const
	{
		return m_PendingKill;
	}

#if WITH_EDITOR
	std::string Actor::GetActorLabel() const
	{
		return ActorLabel;
	}

	void Actor::SetActorLabel(const std::string& InLabel)
	{
		ActorLabel = InLabel;
	}

	void Actor::SetTransient( bool Transient )
	{
		m_Transient = Transient;
	}

#endif
}