#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "Runtime/Engine/EngineTypes.h"

LOG_DECLARE_CATEGORY( LogActor );

namespace Drn
{
	enum class EActorType : uint16
	{
		StaticMeshActor = 0,
		CameraActor
	};

	class Actor : public Serializable
	{
	public:

		Actor();
		~Actor();

		virtual void Tick(float DeltaTime);

		template<typename T>
		void GetComponents(std::vector<T*>& OutComponents, EComponentType Type, bool Recursive)
		{
			return GetRoot()->GetComponents<T>(OutComponents, Type, Recursive);
		}

		Vector GetActorLocation();
		void SetActorLocation(const Vector& InLocation);

		Quat GetActorRotation();
		void SetActorRotation(const Quat& InRotator);

		Vector GetActorScale();
		void SetActorScale(const Vector& InScale);

		Transform GetActorTransform();
		void SetActorTransform( const Transform& InTransform );
		
		void AttachSceneComponent(SceneComponent* InSceneComponent, SceneComponent* Target = nullptr);
		void AddComponent(Component* InComponent);

		SceneComponent* GetRoot() const;

		void Destroy();
		bool IsMarkedPendingKill() const;

		virtual inline EActorType GetActorType() = 0;

		virtual void Serialize(Archive& Ar) override;

#if WITH_EDITOR
		virtual bool IsVisibleInWorldOutliner() const { return true; };
		
		std::string GetActorLabel() const;
		void SetActorLabel(const std::string& InLabel);

		void SetTransient(bool Transient);

		inline bool IsTransient() const { return m_Transient; }
#endif

		void RegisterComponents(World* InWorld);
		void UnRegisterComponents();

		void DispatchPhysicsCollisionHit(const RigidBodyCollisionInfo& MyInfo, const RigidBodyCollisionInfo& OtherInfo, const CollisionImpactData& RigidCollisionData);

	private:

		void RegisterSceneComponentRecursive( SceneComponent* InComponent, World* InWorld );
		void UnRegisterSceneComponentRecursive( SceneComponent* InComponent);

		std::unique_ptr<SceneComponent> Root;

		/** this only contains non scene components */
		std::vector<std::shared_ptr<Component>> Components;

		bool m_PendingKill;

#if WITH_EDITOR
		std::string ActorLabel = "Actor_00";
		bool m_Transient = false;
#endif
	};
}