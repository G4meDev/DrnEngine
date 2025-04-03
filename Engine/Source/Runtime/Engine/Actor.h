#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Actor
	{
	public:

		Actor();
		~Actor();

		virtual void Tick(float DeltaTime);

		DirectX::XMVECTOR GetActorLocation();
		void SetActorLocation(const DirectX::XMVECTOR& InLocation);

		DirectX::XMVECTOR GetActorRotation();
		void SetActorRotation(const DirectX::XMVECTOR& InRotator);

/*
		const Vector3 GetActorScale();
		void SetActorScale(const Vector3& InScale);
*/
		
		void AttachSceneComponent(SceneComponent* InSceneComponent, SceneComponent* Target = nullptr);
		void AddComponent(Component* InComponent);

		SceneComponent* GetRoot() const;

		void Destroy();
		bool IsMarkedPendingKill() const;

#if WITH_EDITOR
		virtual bool IsVisibleInWorldOutliner() const { return true; };
		
		std::string GetActorLabel() const;
		void SetActorLabel(const std::string& InLabel);

		void SetTransient(bool Transient);

		inline bool IsTransient() const { return m_Transient; }
#endif

	private:

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