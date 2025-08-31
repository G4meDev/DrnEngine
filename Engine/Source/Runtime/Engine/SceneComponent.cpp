#include "DrnPCH.h"
#include "SceneComponent.h"
#include "Runtime/Components/BillboardComponent.h"

#if WITH_EDITOR

#include "imgui.h"

#endif

namespace Drn
{
	SceneComponent::SceneComponent() 
		: Component() 
		, m_AttachSocketName("")
	{

	}

	SceneComponent::~SceneComponent()
	{
		
	}

	void SceneComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);

		for (auto SceneComp : Childs)
		{
			SceneComp->Tick(DeltaTime);
		}
	}

	void SceneComponent::AttachSceneComponent(SceneComponent* InComponent)
	{
		InComponent->Parent = this;
		InComponent->SetOwningActor(GetOwningActor());
		Childs.push_back(InComponent);
	}

	std::vector<SceneComponent*> SceneComponent::GetChilds() const
	{
		return Childs;
	}

	void SceneComponent::GetComponentsInline( std::vector<Component*>& OutComponents )
	{
		OutComponents.push_back(this);

		for (SceneComponent* Child : Childs)
		{
			if (Child)
			{
				Child->GetComponentsInline(OutComponents);
			}
		}
	}

	void SceneComponent::Serialize( Archive& Ar )
	{
		Component::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Transform T;
			Ar >> T;
			SetRelativeTransform(T);
		}

		else
		{
			Ar << RelativeTransform;
		}
	}

	void SceneComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

#if WITH_EDITOR
		if (HasSprite())
		{
			m_Sprite = std::make_unique<BillboardComponent>();
			AttachSceneComponent(m_Sprite.get());
			//m_Sprite->RegisterComponent( InOwningWorld );

			m_Sprite->UpdateCachedTransform( true );
		}
#endif
	}

	void SceneComponent::UnRegisterComponent()
	{
		Component::UnRegisterComponent();


	}

	Transform SceneComponent::GetRelativeTransform() const
	{
		return RelativeTransform;
	}

	Transform SceneComponent::GetWorldTransform() const
	{
		return CachedWorldTransform;
	}

	void SceneComponent::SetRelativeTransform( const Transform& InTransform )
	{
		SetRelativeLocationAndRotation(InTransform.GetLocation(), InTransform.GetRotation());
		SetRelativeScale(InTransform.GetScale());
	}

	void SceneComponent::SetWorldTransform( const Transform& InTransform )
	{
		Transform NewRelativeTransform;

		if (Parent)
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());
			NewRelativeTransform = InTransform.GetRelativeTransform(ParentToWorld);
		}

		else
		{
			NewRelativeTransform = InTransform;
		}

		SetRelativeTransform(NewRelativeTransform);
	}

	Vector SceneComponent::GetRelativeLocation() const 
	{
		return RelativeTransform.GetLocation();
	}

	Vector SceneComponent::GetWorldLocation() const 
	{
		return CachedWorldTransform.GetLocation();
	}

	void SceneComponent::SetRelativeLocation( const Vector& Inlocation )
	{
		SetRelativeLocationAndRotation(Inlocation, RelativeTransform.GetRotation());
	}

	void SceneComponent::SetWorldLocation( const Vector& Inlocation )
	{
		Vector NewLocation = Inlocation;
		
		if (Parent)
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());
			NewLocation = ParentToWorld.InverseTransformPosition(NewLocation);
		}

		SetRelativeLocation(NewLocation);
	}

	Quat SceneComponent::GetRelativeRotation() const
	{
		return RelativeTransform.GetRotation();
	}

	Quat SceneComponent::GetWorldRotation() const
	{
		return CachedWorldTransform.GetRotation();
	}

	void SceneComponent::SetRelativeRotation( const Quat& InRotator )
	{
		SetRelativeLocationAndRotation(RelativeTransform.GetLocation(), InRotator);
	}

	void SceneComponent::SetWorldRotation( const Quat& InRotator )
	{
		Quat NewRotation = InRotator;
		
		if (Parent)
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());
			NewRotation = ParentToWorld.GetRotation().Inverse() * NewRotation;
		}

		SetRelativeRotation(NewRotation);
	}

	//void SceneComponent::AddRelativeRotation( const Quat& InRotator )
	//{
	//	SetRelativeLocationAndRotation(GetRelativeLocation(), InRotator * GetRelativeRotation());
	//}

	// void SceneComponent::AddWorldRotation( const Quat& InRotator )
	//{
	//	SetWorldRotation(GetWorldTransform().GetRotation() * InRotator);
	//}

	Vector SceneComponent::GetRelativeScale() const
	{
		return RelativeTransform.GetScale();
	}

	Vector SceneComponent::GetWorldScale() const
	{
		return CachedWorldTransform.GetScale();
	}

	void SceneComponent::SetRelativeScale( const Vector& InScale )
	{
		RelativeTransform.SetScale(InScale);
		UpdateCachedTransform( false );
	}

	void SceneComponent::SetWorldScale( const Vector& InScale )
	{
		Vector NewRelScale = InScale;

		if( Parent )
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());
			NewRelScale = ParentToWorld.GetSafeScaleReciprocal(ParentToWorld.GetScale()) * InScale;
		}

		SetRelativeScale(NewRelScale);
	}

	void SceneComponent::SetRelativeLocationAndRotation( const Vector& InLocation, const Quat& InRotation )
	{
		RelativeTransform.SetLocation(InLocation);
		RelativeTransform.SetRotation(InRotation);
		UpdateCachedTransform( false );
	}

	void SceneComponent::SetWorldLocationAndRotation( const Vector& InLocation, const Quat& InRotation )
	{
		Vector Location = InLocation;
		Quat Rotation = InRotation;

		if (Parent)
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());

			Location = ParentToWorld.InverseTransformPosition(InLocation);
			Rotation = ParentToWorld.GetRotation().Inverse() * InRotation;
		}

		SetRelativeLocationAndRotation(Location, Rotation);
	}

	void SceneComponent::SetWorldLocationAndRotation_SkipPhysic( const Vector& InLocation, const Quat& InRotation )
	{
		Vector Location = InLocation;
		Quat Rotation = InRotation;

		if (Parent)
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());

			Location = ParentToWorld.InverseTransformPosition(InLocation);
			Rotation = ParentToWorld.GetRotation().Inverse() * InRotation;
		}

		RelativeTransform.SetLocation(Location);
		RelativeTransform.SetRotation(Rotation);
		UpdateCachedTransform( true );
	}

	Transform SceneComponent::GetSocketTransform( const std::string& SocketName, ERelativeTransformSpace TransformSpace ) const
	{
		switch ( TransformSpace )
		{
		case ERelativeTransformSpace::Actor:
			return GetWorldTransform().GetRelativeTransform(GetOwningActor()->GetActorTransform());
		case ERelativeTransformSpace::Component:
			return Transform::Identity;
		default:
			return GetWorldTransform();
		}
	}

	Vector SceneComponent::GetForwardVector() const
	{
		return GetWorldTransform().TransformVectorNoScale(Vector::ForwardVector);
	}

	Vector SceneComponent::GetUpVector() const
	{
		return GetWorldTransform().TransformVectorNoScale(Vector::UpVector);
	}

	Vector SceneComponent::GetRightVector() const
	{
		return GetWorldTransform().TransformVectorNoScale(Vector::RightVector);
	}

	void SceneComponent::DestroyComponent()
	{
		//Component::DestroyComponent();


	}

	void SceneComponent::OnUpdateTransform( bool SkipPhysic )
	{
		OnTransformUpdateDel.Braodcast(this, SkipPhysic);
	}

	void SceneComponent::UpdateCachedTransform( bool SkipPhysic )
	{
		Transform NewTransform = CalcNewWorldTransform(RelativeTransform);
		bool HasChanged = !CachedWorldTransform.Equals(NewTransform);

		if (HasChanged)
		{
			CachedWorldTransform = NewTransform;
			PropagateTransformUpdate( SkipPhysic );
		}
	}

	Transform SceneComponent::CalcNewWorldTransform( const Transform& InRelativeTransform ) const
	{
		if (Parent)
		{
			const Transform ParentToWorld = Parent->GetSocketTransform(GetAttachSocketName());
			return InRelativeTransform * ParentToWorld;
		}

		return InRelativeTransform;
	}

	void SceneComponent::PropagateTransformUpdate( bool SkipPhysic )
	{
		// TODO: mark render dirty etc
		OnUpdateTransform(SkipPhysic);

		UpdateChildTransforms( SkipPhysic );
	}

	void SceneComponent::UpdateChildTransforms( bool SkipPhysic )
	{
		for (auto Child : GetChilds())
		{
			Child->UpdateCachedTransform( SkipPhysic );
		}
	}

// -------------------------------------------------------------------------------------------

#if WITH_EDITOR
	void SceneComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

// location
// -------------------------------------------------------------------------------------------

		float TempVector[4];
		TempVector[0] = RelativeTransform.GetLocation().GetX();
		TempVector[1] = RelativeTransform.GetLocation().GetY();
		TempVector[2] = RelativeTransform.GetLocation().GetZ();

		ImGui::DragFloat3( "Location", TempVector );
		Vector NewLocation = Vector( TempVector[0], TempVector[1], TempVector[2]);

// rotation
// -------------------------------------------------------------------------------------------
		
		TempVector[0] = RelativeTransform.GetRotation().GetX();
		TempVector[1] = RelativeTransform.GetRotation().GetY();
		TempVector[2] = RelativeTransform.GetRotation().GetZ();
		TempVector[3] = RelativeTransform.GetRotation().GetW();

		ImGui::DragFloat4( "Rotation", TempVector, 0.01f);
		Quat NewRotation = Quat( TempVector[0], TempVector[1], TempVector[2], TempVector[3]);

		NewRotation = NewRotation.Normalize();

// scale
// -------------------------------------------------------------------------------------------

		TempVector[0] = RelativeTransform.GetScale().GetX();
		TempVector[1] = RelativeTransform.GetScale().GetY();
		TempVector[2] = RelativeTransform.GetScale().GetZ();

		ImGui::DragFloat3( "Scale", TempVector );
		Vector NewScale = Vector( TempVector[0], TempVector[1], TempVector[2] );
	
// -------------------------------------------------------------------------------------------

		SetRelativeTransform( Transform(NewLocation, NewRotation, NewScale) );

		ImGui::Separator();
	}

	void SceneComponent::SetSelectedInEditor( bool SelectedInEditor )
	{
		Component::SetSelectedInEditor(SelectedInEditor);

		
	}

#endif
}