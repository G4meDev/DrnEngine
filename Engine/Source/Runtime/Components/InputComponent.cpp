#include "DrnPCH.h"
#include "InputComponent.h"

#include <imgui.h>

namespace Drn
{
	InputComponent::InputComponent()
		: Component()
	{
	}

	InputComponent::~InputComponent()
	{
	}

	void InputComponent::Tick( float DeltaTime )
	{
		Component::Tick(DeltaTime);


	}

	void InputComponent::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			
		}

		else
		{
			
		}
	}

	void InputComponent::RegisterComponent( World* InOwningWorld )
	{
		Component::RegisterComponent(InOwningWorld);

	}
	
	void InputComponent::UnRegisterComponent()
	{
		

		Component::UnRegisterComponent();
	}
	
	void InputComponent::DestroyComponent()
	{
		Component::DestroyComponent();


	}

#if WITH_EDITOR
	void InputComponent::DrawDetailPanel( float DeltaTime )
	{
		Component::DrawDetailPanel(DeltaTime);

		ImGui::Text( "ewrawer" );
	}
#endif
}