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

/*
		DirectX::XMVECTOR GetActorLocation();
		void SetActorLocation(const DirectX::XMVECTOR& InLocation);
*/

/*
		const Quaternion GetActorRotation();
		void SetActorRotation(const Quaternion& InRotator);

		const Vector3 GetActorScale();
		void SetActorScale(const Vector3& InScale);
*/
		
		void AttachSceneComponent(SceneComponent* InSceneComponent, SceneComponent* Target = nullptr);
		void AddComponent(Component* InComponent);

		SceneComponent* GetRoot() const;

		void MarkDestroy();
		bool IsMarkDestroy() const;

#if WITH_EDITOR
		virtual bool IsVisibleInWorldOutliner() const { return true; };
		
		std::string GetActorLabel() const;
		void SetActorLabel(const std::string& InLabel);
#endif

	private:

		std::unique_ptr<SceneComponent> Root;

		/** this only contains non scene components */
		std::vector<Component*> Components;

		bool bDestroy = false;

#if WITH_EDITOR
		std::string ActorLabel = "Actor_00";
#endif

	};
}