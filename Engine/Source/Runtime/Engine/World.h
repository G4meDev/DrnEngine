#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class World
	{
	public:

		World();
		~World();

		void Tick(float DeltaTime);

		template<typename T>
		T* SpawnActor()
		{
			T* NewActor = new T();
			m_Actors.insert(NewActor);

			return NewActor;
		}

	protected:

		std::set<Actor*> m_Actors;

		friend Scene;

	private:
	};
}