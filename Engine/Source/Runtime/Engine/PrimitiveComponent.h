#pragma once

#include "ForwardTypes.h"
#include "SceneComponent.h"

namespace Drn
{
	class PrimitiveComponent : public SceneComponent
	{
	public:
		PrimitiveComponent();
		virtual ~PrimitiveComponent();

		virtual void Serialize( Archive& Ar ) override;

	protected:

	private:
	};

}