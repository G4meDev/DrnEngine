#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"

namespace Drn
{
	class SpringArmComponent : public SceneComponent
	{
	public:
		SpringArmComponent();
		virtual ~SpringArmComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::SpringArmComponent; }
		inline static EComponentType GetComponentTypeStatic() { return EComponentType::SpringArmComponent; }

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		virtual Transform GetSocketTransform( const std::string& SocketName,
			ERelativeTransformSpace TransformSpace = ERelativeTransformSpace::World ) const override;

		virtual void RegisterComponent(World* InOwningWorld) override;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;

		void DrawArm();
		void DrawEditorDefault() override;
		void DrawEditorSelected() override;
#endif

	protected:

		void UpdateDesiredLocationAndRotation(float DeltaTime, bool LocationLag, bool RotationLag);

		float m_ArmLength;
		bool m_LocationLag;
		float m_LocationLagSpeed;
		bool m_RotationLag;
		float m_RotationLagSpeed;

		Vector m_PreviousDesiredLocation;
		Quat m_PreviousDesiredRotation;

		Vector m_RelativeSocketLocation;
		Quat m_RelativeSocketRotation;
	};
}