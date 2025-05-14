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

		std::vector<std::shared_ptr<SceneComponent>> GetChilds() const;

		// TODO: make type as a static function
		template<typename T>
		void GetComponents(std::vector<T*>& OutComponents, EComponentType Type, bool Recursive)
		{
			// TODO: add self to type check
			for (auto Comp : Childs)
			{
				//if (Comp->GetComponentType() == Type)
				//{
					T* C = static_cast<T*>(Comp.get());
					if (C)
					{
						OutComponents.push_back( static_cast<T*>(Comp.get()) );
					}

					if (Recursive)
					{
						Comp->GetComponents(OutComponents, Type, true);
					}
				//}
			}
		}

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;


#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
#endif

		Transform GetRelativeTransform();
		Transform GetWorldTransform();

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

	private:

		Transform RelativeTransform;
		Transform CachedWorldTransform;

		void UpdateCachedTransform();
		Transform CalcNewWorldTransform(const Transform& InRelativeTransform) const;

		void PropagateTransformUpdate();
		void UpdateChildTransforms();


		SceneComponent* Parent = nullptr;
		std::vector<std::shared_ptr<SceneComponent>> Childs;
	};
}