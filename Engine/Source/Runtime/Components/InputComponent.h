#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/Component.h"

namespace Drn
{
	class InputComponent : public Component
	{
	public:
		InputComponent();
		virtual ~InputComponent();

		void Tick( float DeltaTime ) override;
		EComponentType GetComponentType() override { return EComponentType::InputComponent; };
		void Serialize( Archive& Ar ) override;
		void DrawDetailPanel( float DeltaTime ) override;
		void RegisterComponent( World* InOwningWorld ) override;
		void UnRegisterComponent() override;
		void DestroyComponent() override;

	protected:

	};
}