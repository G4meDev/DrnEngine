#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class World
	{
	public:

		World();
		~World();

		void AddPrimitive(PrimitiveComponent* InPrimitive);
		void RemovePrimitive(PrimitiveComponent* InPrimitive);

	protected:

		

		// @TODO: promote to actor
		std::vector<PrimitiveComponent*> m_PrimitiveComponents;

		friend Scene;

	private:
	};
}