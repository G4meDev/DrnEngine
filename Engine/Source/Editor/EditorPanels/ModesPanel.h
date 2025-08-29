#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"

namespace Drn
{
	class ModesPanel
	{
	public:
		ModesPanel();
		~ModesPanel();

		void Draw(float DeltaTime);

		std::string m_SelectedCateory;

	protected:

	};
}

#endif