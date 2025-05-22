#pragma once

#include "ForwardTypes.h"
#include "Component.h"

namespace Drn
{
	enum class EUpdateTransformFlags : uint8
	{
		None,
		SkipPhysicUpdate,
		PropagateFromParent
	};

	class SceneComponent : public Component
	{
	public:
		SceneComponent();
		virtual ~SceneComponent();
		
		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::SceneComponenet; }

		void AttachSceneComponent(SceneComponent* InComponent);

		inline bool HasChild() const { return Childs.size() > 0; }

		std::vector<SceneComponent*> GetChilds() const;

		// TODO: make type as a static function
		// TODO: make this only recursive
		template<typename T>
		void GetComponents(std::vector<T*>& OutComponents, EComponentType Type, bool Recursive)
		{
			//if (Comp->GetComponentType() == Type)
			T* C = static_cast<T*>(this);
			if (C)
			{
				OutComponents.push_back( C );
			}

			for (auto Comp : Childs)
			{
				Comp->GetComponents(OutComponents, Type, true);
			}
		}

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;

		inline virtual bool HasSprite() const { return false; }
#endif

		Transform GetRelativeTransform() const;
		Transform GetWorldTransform() const;

		void SetRelativeTransform(const Transform& InTransform);
		void SetWorldTransform(const Transform& InTransform);

		Vector GetRelativeLocation() const;
		Vector GetWorldLocation() const;
		
		void SetRelativeLocation(const Vector& Inlocation);
		void SetWorldLocation(const Vector& Inlocation);

		Quat GetRelativeRotation() const;
		Quat GetWorldRotation() const;

		void SetRelativeRotation(const Quat& InRotator);
		void SetWorldRotation(const Quat& InRotator);
		
		Vector GetRelativeScale() const;
		Vector GetWorldScale() const;
		
		void SetRelativeScale(const Vector& InScale);
		void SetWorldScale(const Vector& InScale);

		void SetRelativeLocationAndRotation(const Vector& InLocation, const Quat& InRotation);
		void SetWorldLocationAndRotation(const Vector& InLocation, const Quat& InRotation);
		void SetWorldLocationAndRotation_SkipPhysic( const Vector& InLocation, const Quat& InRotation );

		inline SceneComponent* GetParent() const { return Parent; }

	protected:

		virtual void OnUpdateTransform( bool SkipPhysic );

#if WITH_EDITOR
		std::unique_ptr<class BillboardComponent> m_Sprite = nullptr;
#endif

	private:

		Transform RelativeTransform;
		Transform CachedWorldTransform;

		void UpdateCachedTransform( bool SkipPhysic );
		Transform CalcNewWorldTransform(const Transform& InRelativeTransform) const;

		void PropagateTransformUpdate( bool SkipPhysic );
		void UpdateChildTransforms( bool SkipPhysic );

		SceneComponent* Parent = nullptr;
		std::vector<SceneComponent*> Childs;

		friend class Actor;
	};
}