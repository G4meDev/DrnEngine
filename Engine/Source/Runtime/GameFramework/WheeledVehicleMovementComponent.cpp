#include "DrnPCH.h"
#include "WheeledVehicleMovementComponent.h"

namespace Drn
{
	WheeledVehicleMovementComponent::WheeledVehicleMovementComponent()
		: Component()
		, Mass(1000.0f)
	{
	
	}

	WheeledVehicleMovementComponent::~WheeledVehicleMovementComponent()
	{
		
	}

	void WheeledVehicleMovementComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);


	}

	void WheeledVehicleMovementComponent::Serialize( Archive& Ar )
	{
		Component::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Mass;
		}
		else
		{
			Ar << Mass;
		}
	}

	void WheeledVehicleMovementComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

	}

	void WheeledVehicleMovementComponent::UnRegisterComponent()
	{
		Component::UnRegisterComponent();
		
	}

#if WITH_EDITOR
	void WheeledVehicleMovementComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

		ImGui::InputFloat("Mass", &Mass);
	}
#endif

}  // namespace Drn