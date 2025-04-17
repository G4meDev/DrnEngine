#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

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

		Vector GetActorLocation();
		void SetActorLocation(const Vector& InLocation);

		Quat GetActorRotation();
		void SetActorRotation(const Quat& InRotator);

		Vector GetActorScale();
		void SetActorScale(const Vector& InScale);

		
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