#pragma once

#include "ForwardTypes.h"
#include "SceneComponent.h"

#include "Runtime/Physic/BodyInstance.h"

namespace Drn
{
	class PrimitiveComponent : public SceneComponent
	{
	public:
		PrimitiveComponent();
		virtual ~PrimitiveComponent();

		virtual void Serialize( Archive& Ar ) override;

		inline BodyInstance GetBodyInstance() { return m_BodyInstance; }

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

	protected:

		BodyInstance m_BodyInstance;

	private:
	};

}