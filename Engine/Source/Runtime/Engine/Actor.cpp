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

			uint8 ActorTagsCount;
			Ar >> ActorTagsCount;

			Tags.resize(ActorTagsCount);

			for (uint8 TagIndex = 0; TagIndex < ActorTagsCount; TagIndex++)
			{
				Ar >> Tags[TagIndex];
			}
		}

#if WITH_EDITOR

		else
		{
			Ar << ActorLabel;

			const uint32 ActorTagsCount = Tags.size();
			drn_check(ActorTagsCount < UINT8_MAX)
			Ar << static_cast<uint8>(ActorTagsCount);

			for (uint8 TagIndex = 0; TagIndex < ActorTagsCount; TagIndex++)
			{
				Ar << Tags[TagIndex];
			}
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

	void Actor::DrawEditorDefault()
	{
		std::vector<Component*> Comps;
		GetComponentsInline(Comps);

		for (Component* Comp : Comps)
		{
			Comp->DrawEditorDefault();
		}
	}

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

	bool Actor::DrawDetailPanel()
	{
		bool bDirty = false;

		if (ImGui::CollapsingHeader("Tags"))
		{
			ImGui::PushID("Tags");
			if (ImGui::Button("Add"))
			{
				AddActorTag("");
				bDirty = true;
			}

			if (ImGui::Button("Remove"))
			{
				if (!Tags.empty())
				{
					Tags.pop_back();
					bDirty = true;
				}
			}

			if (ImGui::Button("Clear"))
			{
				Tags.clear();
				bDirty = true;
			}

			const int32 NumTags = Tags.size();
			for (int32 Index = 0; Index < NumTags; Index++)
			{
				char Tag[128];
				strcpy(Tag, Tags[Index].c_str());

				ImGui::PushID(Index);
				if (ImGui::InputText("##", Tag, 128))
				{
					Tags[Index] = Tag;
					bDirty = true;
				}
				ImGui::PopID();
			}
			ImGui::PopID();
		}

		return bDirty;
	}

#endif

	void Actor::RegisterComponents( World* InWorld )
	{
		RegisterSceneComponentRecursive(Root, InWorld);

		for (auto Comp : Components)
		{
			if(!Comp->IsRegistered())
				Comp->RegisterComponent(InWorld);
		}
	}

	void Actor::UnRegisterComponents()
	{
		UnRegisterSceneComponentRecursive(Root);

		for (auto Comp : Components)
		{
			if(Comp->IsRegistered())
				Comp->UnRegisterComponent();
		}
	}

	void Actor::PostInitializeComponents()
	{
		
	}

	void Actor::DispatchPhysicsCollisionHit( const RigidBodyCollisionInfo& MyInfo,
		const RigidBodyCollisionInfo& OtherInfo, const CollisionImpactData& RigidCollisionData )
	{
		for ( const RigidBodyContactInfo& ContactInfo : RigidCollisionData.ContactInfos )
		{
			Root->GetWorld()->DrawDebugSphere( ContactInfo.ContactPosition, Quat::Identity, Color::Blue, 0.8f, 8, 0.05, 0 );
			Root->GetWorld()->DrawDebugLine( ContactInfo.ContactPosition, ContactInfo.ContactPosition + ContactInfo.ContactNormal * 0.5, Color::Blue, 0.05f, 0 );
		}

		//std::cout << (uint32)RigidCollisionData.ContactInfos[0].PhysMaterial[1]->GetSurfaceType() << "\n";

		const RigidBodyContactInfo& ContactInfo = RigidCollisionData.ContactInfos[0];
		
		HitResult Result;
		Result.Location = ContactInfo.ContactPosition;
		Result.Normal = ContactInfo.ContactNormal;
		Result.PhysMaterial = ContactInfo.PhysMaterial[1];
		Result.HitActor = OtherInfo.m_Actor;
		Result.HitComponent = OtherInfo.m_Component;
		//Result.Item = OtherInfo.BodyIndex;
		//Result.BoneName = OtherInfo.BoneName;
		//Result.MyBoneName = MyInfo.BoneName;
		Result.bBlockingHit = true;
		
		if (OnActorHitDel.IsBound())
		{
			OnActorHitDel.Braodcast(this, OtherInfo.m_Actor, RigidCollisionData.TotalNormalImpulse, Result);
		}
		
		PrimitiveComponent* MyInfoComponent = MyInfo.m_Component;
		if (MyInfoComponent && MyInfoComponent->OnComponentHitDel.IsBound())
		{
			MyInfoComponent->OnComponentHitDel.Braodcast(MyInfoComponent, OtherInfo.m_Actor, OtherInfo.m_Component, RigidCollisionData.TotalNormalImpulse, Result);
		}

	}

	bool Actor::ActorHasTag( const std::string& Tag ) const
	{
		return std::find(Tags.begin(), Tags.end(), Tag) != Tags.end();
	}

	void Actor::AddActorTag( const std::string& Tag )
	{
		if (!ActorHasTag(Tag))
		{
			Tags.push_back(Tag);
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