#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

namespace Drn
{
	enum class EComponentType : uint32
	{
		Component = 0,
		SceneComponenet,
		StaticMeshComponent,
		CameraComponent,
	};

	class Component : public Serializable
	{
	public:
		Component();
		virtual ~Component();

		virtual void Tick(float DeltaTime);

		inline virtual EComponentType GetComponentType() { return EComponentType::Component; }

		Actor* GetOwningActor() const;
		void SetOwningActor(Actor* InActor);

		bool IsActive() const;
		void SetActive(bool Active);

		virtual void Serialize( Archive& Ar );

#if WITH_EDITOR
		std::string GetComponentLabel() const;
		void SetComponentLabel(const std::string& InLabel);

		virtual void DrawDetailPanel(float DeltaTime) {};
#endif

	private:
		Actor* Owner = nullptr;

		bool bActive = true;

#if WITH_EDITOR
		std::string ComponentLabel = "Component_00";
#endif
	};
}