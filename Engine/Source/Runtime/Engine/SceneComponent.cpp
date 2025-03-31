#include "DrnPCH.h"
#include "SceneComponent.h"

namespace Drn
{
	SceneComponent::SceneComponent() : Component()
	{
		
	}

	void SceneComponent::Tick(float DeltaTime)
	{
		Component::Tick(DeltaTime);

		UpdateLocation();
		//UpdateRotation();
		//UpdateScale();

		for (SceneComponent* SceneComp : Childs)
		{
			SceneComp->Tick(DeltaTime);
		}
	}

	void SceneComponent::AttachSceneComponent(SceneComponent* InComponent)
	{
		InComponent->Parent = this;
		Childs.push_back(InComponent);
	}

	const std::vector<SceneComponent*>& SceneComponent::GetChilds() const
	{
		return Childs;
	}

	DirectX::XMVECTOR SceneComponent::GetRelativeLocation() const
	{
		return RelativeLocation;
	}

	DirectX::XMVECTOR SceneComponent::GetLocalLocation() const
	{
		return LocalLocation;
	}

	DirectX::XMVECTOR SceneComponent::GetWorldLocation()const
	{
		return WorldLocation;
	}

	void SceneComponent::SetRelativeLocation( const DirectX::XMVECTOR& Inlocation, bool bMarkDirty )
	{
		RelativeLocation = Inlocation;
		if (bMarkDirty)
		{
			MarkDirtyLocationRecursive();
		}
	}

	void SceneComponent::SetLocalLocation( const DirectX::XMVECTOR& Inlocation, bool bMarkDirty)
	{

	}

	void SceneComponent::SetWorldLocation( const DirectX::XMVECTOR& Inlocation, bool bMarkDirty )
	{
		WorldLocation = Inlocation;

		//if (GetOwningActor()->GetRoot() == this)
		//{
		//	WorldLocation = Inlocation;
		//	RelativeLocation = Inlocation;
		//}
		//
		//if (bMarkDirty)
		//{
		//	MarkDirtyLocationRecursive();
		//}
	}

	void SceneComponent::UpdateLocation()
	{
		if (bDirtyLocation)
		{
			//WorldLocation = RelativeLocation + Parent->GetWorldLocation();
			//LocalLocation = WorldLocation - GetOwningActor()->GetActorLocation();
			
			bDirtyLocation = false;
		}
	}

	bool SceneComponent::IsDirtyLocation() const
	{
		return bDirtyLocation;
	}

	void SceneComponent::MarkDirtyLocation()
	{
		bDirtyLocation = true;
	}

	void SceneComponent::MarkDirtyLocationRecursive()
	{
		MarkDirtyLocation();

		for (SceneComponent* Comp : Childs)
		{
			Comp->MarkDirtyLocationRecursive();
		}
	}

/*
	const Quaternion SceneComponent::GetRelativeRotation() const
	{
		return RelativeRotation;
	}

	const Quaternion SceneComponent::GetLocalRotation() const
	{
		return LocalRotation;
	}

	const Quaternion SceneComponent::GetWorldRotation() const
	{
		return WorldRotation;
	}

	void SceneComponent::SetRelativeRotation(const Quaternion& InRotator, bool bMarkDirty)
	{
		RelativeRotation = InRotator;
		if (bMarkDirty)
		{
			MarkDirtyRotationRecursive();
		}
	}

	void SceneComponent::SetLocalRotation(const Quaternion& InRotator, bool bMarkDirty)
	{

	}

	void SceneComponent::SetWorldRotation(const Quaternion& InRotator, bool bMarkDirty)
	{
		if (GetOwningActor()->GetRoot() == this)
		{
			WorldRotation = InRotator;
			RelativeRotation = InRotator;
		}

		MarkDirtyRotationRecursive();
	}

	void SceneComponent::UpdateRotation()
	{
		if (bDirtyRotation)
		{
			WorldRotation = RelativeRotation + Parent->GetWorldRotation();
			LocalRotation = Rotatorf::CombineRotators(WorldRotation, GetOwningActor()->GetActorRotation() * -1);

			bDirtyRotation = false;
		}
	}

	bool SceneComponent::IsDirtyRotation() const
	{
		return bDirtyRotation;
	}

	void SceneComponent::MarkDirtyRotation()
	{
		bDirtyRotation = true;
	}

	void SceneComponent::MarkDirtyRotationRecursive()
	{
		MarkDirtyRotation();

		for (SceneComponent* Comp : Childs)
		{
			Comp->MarkDirtyRotationRecursive();
		}
	}

	const Vector3 SceneComponent::GetRelativeScale() const
	{
		return RelativeScale;
	}

	const Vector3 SceneComponent::GetLocalScale() const
	{
		return LocalScale;
	}

	const Vector3 SceneComponent::GetWorldScale() const
	{
		return WorldScale;
	}

	void SceneComponent::SetRelativeScale(const Vector3& InScale, bool bMarkDirty)
	{
		RelativeScale = InScale;

		if (bMarkDirty)
		{
			MarkDirtyScaleRecursive();
		}
	}

	void SceneComponent::SetLocalScale(const Vector3& InScale, bool bMarkDirty)
	{

	}

	void SceneComponent::SetWorldScale(const Vector3& InScale, bool bMarkDirty)
	{
		if (GetOwningActor()->GetRoot() == this)
		{
			WorldScale = InScale;
			RelativeScale = InScale;
		}

		MarkDirtyScaleRecursive();
	}

	void SceneComponent::UpdateScale()
	{
		if (bDirtyScale)
		{
			WorldScale = RelativeScale * Parent->GetWorldScale();
			LocalScale = WorldScale / GetOwningActor()->GetActorScale();

			bDirtyScale = false;
		}
	}

	bool SceneComponent::IsDirtyScale() const
	{
		return bDirtyScale;
	}

	void SceneComponent::MarkDirtyScale()
	{
		bDirtyScale = true;
	}

	void SceneComponent::MarkDirtyScaleRecursive()
	{
		MarkDirtyScale();

		for (SceneComponent* Comp : Childs)
		{
			Comp->MarkDirtyScaleRecursive();
		}
	}

*/
}