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

		inline void SetTickEnabled( bool Enabled ) { m_ShouldTick = Enabled; }

		template<typename T>
		T* SpawnActor()
		{
			T* NewActor = new T();
			m_Actors.insert(NewActor);

			return NewActor;
		}

		inline const std::set<Actor*>& GetActorList() { return m_Actors; };

	protected:

		std::set<Actor*> m_Actors;

		bool m_ShouldTick;

		friend Scene;

	private:
	};
}