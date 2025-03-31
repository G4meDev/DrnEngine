#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Component
	{
	public:
		Component();

		virtual void Tick(float DeltaTime);

		Actor* GetOwningActor() const;
		void SetOwningActor(Actor* InActor);

		bool IsActive() const;
		void SetActive(bool Active);

#if WITH_EDITOR
		std::string GetComponentLabel() const;
		void SetComponentLabel(const std::string& InLabel); 
#endif

	private:
		Actor* Owner = nullptr;

		bool bActive = true;

#if WITH_EDITOR
		std::string ComponentLabel = "Component_00";
#endif
	};
}