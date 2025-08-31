#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/LightActor.h"
#include "Runtime/Components/DirectionalLightComponent.h"

namespace Drn
{
	class DirectionalLightActor : public LightActor
	{
	public:
		DirectionalLightActor();
		virtual ~DirectionalLightActor();

#if WITH_EDITOR
		virtual void DrawEditorSelected() override;
#endif

	protected:
		virtual EActorType GetActorType() override { return EActorType::DirectionalLight; }
		std::unique_ptr<DirectionalLightComponent> m_DirectionalLightComponent;
	};
}