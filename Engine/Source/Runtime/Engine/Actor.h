#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "Runtime/Engine/EngineTypes.h"

LOG_DECLARE_CATEGORY( LogActor );

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam(OnActorDestroyedDelegate, Actor*);

	enum class EActorType : uint16
	{
		StaticMeshActor = 0,
		CameraActor,
		PointLight,
		SpotLight,
		PostProcessVolume,
		DirectionalLight,
		SkyLight,
		Pawn,
		Controller,
		PlayerController,
		CameraManager
	};

	class Actor : public Serializable
	{
	public:

		Actor();
		virtual ~Actor();

		virtual void Tick(float DeltaTime);

		template<typename T>
		void GetComponents(std::vector<T*>& OutComponents, EComponentType Type, bool Recursive)
		{
			OutComponents = Components;
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
		
		inline void SetRootComponent( SceneComponent* InRootComponent )
		{
			Root = InRootComponent;
			Root->SetOwningActor(this);
		}

		void AttachSceneComponent(SceneComponent* InSceneComponent, SceneComponent* Target = nullptr);
		void AddComponent(Component* InComponent);

		SceneComponent* GetRoot() const;
		inline World* GetWorld() const { return m_World; }

		void Destroy();
		bool IsMarkedPendingKill() const;

		virtual inline EActorType GetActorType() = 0;

		virtual void Serialize(Archive& Ar) override;

		OnActorDestroyedDelegate OnActorKilled;

#if WITH_EDITOR
		virtual bool IsVisibleInWorldOutliner() const { return true; };
		
		std::string GetActorLabel() const;
		void SetActorLabel(const std::string& InLabel);

		void SetTransient(bool Transient);

		inline bool IsTransient() const { return m_Transient; }

		void SetComponentsSelectedInEditor( bool SelectedInEditor );

		virtual bool DrawDetailPanel() { return false; };

#else
		void SetTransient(bool Transient){};
#endif

		void RegisterComponents(World* InWorld);
		void UnRegisterComponents();

		void DispatchPhysicsCollisionHit(const RigidBodyCollisionInfo& MyInfo, const RigidBodyCollisionInfo& OtherInfo, const CollisionImpactData& RigidCollisionData);

	private:

		void RegisterSceneComponentRecursive( SceneComponent* InComponent, World* InWorld );
		void UnRegisterSceneComponentRecursive( SceneComponent* InComponent);

		void RemoveOwnedComponent(Component* InComponent);

		//std::unique_ptr<SceneComponent> Root;
		SceneComponent* Root;

		/** this only contains non scene components */
		std::vector<Component*> Components;

		bool m_PendingKill;
		World* m_World;

#if WITH_EDITOR
		std::string ActorLabel = "Actor_00";
		bool m_Transient = false;
#endif

		friend class World;
		friend class Component;
		friend class ActorDetailPanel;
	};
}