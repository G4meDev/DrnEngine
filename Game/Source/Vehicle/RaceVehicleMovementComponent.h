#pragma once

#include "Runtime/GameFramework/WheeledVehicleMovementComponent.h"
#include "ForwardTypes.h"

namespace Drn
{
	class RaceVehicleMovementComponent : public WheeledVehicleMovementComponent
	{
	public:
		RaceVehicleMovementComponent();
		virtual ~RaceVehicleMovementComponent();

		//virtual void Serialize( Archive& Ar ) override;

		virtual EComponentType GetComponentType() override { return static_cast<EComponentType>(EGameComponentType::RaceVehicleMovementComponent); }
		inline static EComponentType GetComponentTypeStatic() { return static_cast<EComponentType>(EGameComponentType::RaceVehicleMovementComponent); };

		//virtual void Tick( float DeltaTime ) override;

#if WITH_EDITOR
		//virtual bool DrawDetailPanel() override;
		//virtual void DrawEditorDefault() override;
		//virtual void DrawEditorSelected() override;
#endif

	protected:

	};
}