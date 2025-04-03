#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class World
	{
	public:

		using OnNewActors = std::function<void( const std::set<Actor*>& )>;
		using OnRemoveActor = std::function<void( const Actor* )>;

		World();
		~World();

		void Tick(float DeltaTime);

		inline void SetTickEnabled( bool Enabled ) { m_ShouldTick = Enabled; }

		template<typename T>
		T* SpawnActor()
		{
			T* NewActor = new T();
			m_NewActors.insert(NewActor);

			return NewActor;
		}

		inline const std::set<Actor*>& GetActorList() { return m_Actors; };


		void BindOnNewActors(OnNewActors Delegate);
		void RemoveFromOnNewActors(OnNewActors Delegate);
		void InvokeOnNewActors(const std::set<Actor*>& NewActors);


		void BindOnRemoveActor(OnRemoveActor Delegate);
		void RemoveFromOnRemoveActor(OnRemoveActor Delegate);
		void InvokeOnRemoveActor(const Actor* RemovedActor);


	protected:

		std::set<Actor*> m_Actors;

		// actors added in middle of frame get added to actor list at start of next frame
		std::set<Actor*> m_NewActors;

		bool m_ShouldTick;

		std::vector<OnNewActors> OnNewActorsDelegates;
		std::vector<OnRemoveActor> OnRemoveActorDelegates;

		friend Scene;

	private:
	};
}