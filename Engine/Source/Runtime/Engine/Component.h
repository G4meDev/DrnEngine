#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "Runtime/Core/Guid.h"

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

		inline World* GetWorld() { return m_OwningWorld; }

		Actor* GetOwningActor() const;
		void SetOwningActor(Actor* InActor);

		bool IsActive() const;
		void SetActive(bool Active);

		virtual void Serialize( Archive& Ar );

		std::string GetComponentLabel() const;
		void SetComponentLabel(const std::string& InLabel);

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) {};
#endif

		virtual void RegisterComponent(World* InOwningWorld);
		virtual void UnRegisterComponent();

		inline const Guid& GetGuid() const { return m_Guid; }

#if WITH_EDITOR
		inline bool IsSelectedInEditor() const { return m_SelectedInEditor; }
		virtual void SetSelectedInEditor( bool SelectedInEditor );

		inline virtual bool ShouldDrawInComponentHeirarchy() const { return true; }
#endif

	protected:
		Guid m_Guid;

#if WITH_EDITOR
		bool m_SelectedInEditor = false;
		std::string ComponentLabel = "Component_00";
#endif

	private:

		World* m_OwningWorld;
		Actor* Owner = nullptr;

		bool bActive = true;

		friend class Actor;
	};
}