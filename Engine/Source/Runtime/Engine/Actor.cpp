#include "DrnPCH.h"
#include "Actor.h"

namespace Drn
{
	Actor::Actor()
	{
		Root = std::make_unique<SceneComponent>();
		Root->SetOwningActor(this);
	}

	Actor::~Actor()
	{
	}

	void Actor::Tick(float DeltaTime)
	{
		for (Component* Comp : Components)
		{
			Comp->Tick(DeltaTime);
		}

		for (SceneComponent* SceneComp : Root->GetChilds())
		{
			SceneComp->Tick(DeltaTime);
		}
	}

/*
	DirectX::XMVECTOR Actor::GetActorLocation()
	{
		return Root->GetWorldLocation();
	}

	void Actor::SetActorLocation( const DirectX::XMVECTOR& InLocation )
	{
		Root->SetWorldLocation(InLocation);
		Root->MarkDirtyLocationRecursive();
	}
*/

/*
	const Quaternion Actor::GetActorRotation()
	{
		return Root->GetWorldRotation();
	}

	void Actor::SetActorRotation(const Quaternion& InRotator)
	{
		Root->SetWorldRotation(InRotator);
		Root->MarkDirtyRotationRecursive();
	}

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
		Components.push_back(InComponent);
		InComponent->SetOwningActor(this);
	}

	SceneComponent* Actor::GetRoot() const
	{
		return Root.get();
	}

	void Actor::MarkDestroy()
	{
		bDestroy = true;
	}

	bool Actor::IsMarkDestroy() const
	{
		return bDestroy;
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
#endif
}