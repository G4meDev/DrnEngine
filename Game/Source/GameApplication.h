#pragma once

#include "Runtime/Core/Application.h"

namespace Drn
{
	class GameApplication : public Application
	{
	public:
	

	protected:
		virtual void Startup() override;
		virtual void Shutdown() override;
	
		virtual void Tick(float DeltaTime) override;

	};
}
