#include "DrnPCH.h"
#include "SceneComponent.h"

#if WITH_EDITOR

#include "imgui.h"

#endif

namespace Drn
{
	SceneComponent::SceneComponent() 
		: Component() 
	{
		WorldLocation = XMVectorSet(0, 0, 0, 0);
		WorldRotation = XMQuaternionIdentity();
	}

	SceneComponent::~SceneComponent()
	{
		
	}

	void SceneComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		UpdateLocation();
		//UpdateRotation();
		//UpdateScale();

		for (auto SceneComp : Childs)
		{
			SceneComp->Tick(DeltaTime);
		}
	}

	void SceneComponent::AttachSceneComponent(SceneComponent* InComponent)
	{
		InComponent->Parent = this;
		InComponent->SetOwningActor(GetOwningActor());
		Childs.push_back(std::shared_ptr<SceneComponent>(InComponent));
	}

	std::vector<std::shared_ptr<SceneComponent>> SceneComponent::GetChilds() const
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
		if (Parent == nullptr)
		{
			return WorldLocation;
		}

		return GetOwningActor()->GetActorLocation();
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

		for (auto Comp : Childs)
		{
			Comp->MarkDirtyLocationRecursive();
		}
	}


	DirectX::XMVECTOR SceneComponent::GetRelativeRotation() const
	{
		return RelativeRotation;
	}

	DirectX::XMVECTOR SceneComponent::GetLocalRotation() const
	{
		return LocalRotation;
	}

	DirectX::XMVECTOR SceneComponent::GetWorldRotation() const
	{
		if (Parent == nullptr)
		{
			return WorldRotation;
		}

		return GetOwningActor()->GetActorRotation();
	}

	void SceneComponent::SetRelativeRotation( const DirectX::XMVECTOR& InRotator, bool bMarkDirty )
	{
		//RelativeRotation = InRotator;
		//if (bMarkDirty)
		//{
		//	MarkDirtyRotationRecursive();
		//}
	}

	void SceneComponent::SetLocalRotation( const DirectX::XMVECTOR& InRotator, bool bMarkDirty )
	{

	}

	void SceneComponent::SetWorldRotation( const DirectX::XMVECTOR& InRotator, bool bMarkDirty )
	{
		WorldRotation = InRotator;

		//if (GetOwningActor()->GetRoot() == this)
		//{
		//	WorldRotation = InRotator;
		//	RelativeRotation = InRotator;
		//}
		//
		//MarkDirtyRotationRecursive();
	}

	void SceneComponent::UpdateRotation()
	{
		//if (bDirtyRotation)
		//{
		//	WorldRotation = RelativeRotation + Parent->GetWorldRotation();
		//	LocalRotation = Rotatorf::CombineRotators(WorldRotation, GetOwningActor()->GetActorRotation() * -1);
		//
		//	bDirtyRotation = false;
		//}
	}

	bool SceneComponent::IsDirtyRotation() const
	{
		return bDirtyRotation;
	}

	void SceneComponent::MarkDirtyRotation()
	{
		//bDirtyRotation = true;
	}

	void SceneComponent::MarkDirtyRotationRecursive()
	{
		//MarkDirtyRotation();
		//
		//for (SceneComponent* Comp : Childs)
		//{
		//	Comp->MarkDirtyRotationRecursive();
		//}
	}

/*

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

// -------------------------------------------------------------------------------------------

#if WITH_EDITOR
	void SceneComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

		float Vector[4];
		Vector[0] = XMVectorGetX(WorldLocation);
		Vector[1] = XMVectorGetY(WorldLocation);
		Vector[2] = XMVectorGetZ(WorldLocation);

		ImGui::DragFloat3( "Location", Vector );
		WorldLocation = XMVectorSet( Vector[0], Vector[1], Vector[2], 0);

// -------------------------------------------------------------------------------------------
		Vector[0] = XMVectorGetX(WorldRotation);
		Vector[1] = XMVectorGetY(WorldRotation);
		Vector[2] = XMVectorGetZ(WorldRotation);
		Vector[3] = XMVectorGetW(WorldRotation);

		ImGui::DragFloat4( "Rotation", Vector, 0.01f);
		WorldRotation = XMVectorSet( Vector[0], Vector[1], Vector[2], Vector[3]);

		WorldRotation = XMQuaternionNormalize(WorldRotation);

		ImGui::Separator();
	}
#endif
}