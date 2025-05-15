#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogWorld);

namespace Drn
{
	class World
	{
	public:

		using OnNewActors = std::function<void( const std::set<Actor*>& )>;
		using OnRemoveActor = std::function<void( const Actor* )>;

		World();
		~World();

		void Destroy();

		void Tick(float DeltaTime);

		inline void SetTickEnabled( bool Enabled ) { m_ShouldTick = Enabled; }

		template<typename T>
		T* SpawnActor()
		{
			T* NewActor = new T();
			m_NewActors.insert(NewActor);

			return NewActor;
		}

		Component* GetComponentWithGuid(const Guid& ID);

		inline const std::set<Actor*>& GetActorList() { return m_Actors; };

		void BindOnNewActors(OnNewActors Delegate);
		void RemoveFromOnNewActors(OnNewActors Delegate);
		void InvokeOnNewActors(const std::set<Actor*>& NewActors);

		void BindOnRemoveActor(OnRemoveActor Delegate);
		void RemoveFromOnRemoveActor(OnRemoveActor Delegate);
		void InvokeOnRemoveActor(const Actor* RemovedActor);

		inline void SetTransient( bool Transient ) { m_Transient = true; }
		inline bool IsTransient() { return m_Transient; }

		inline bool IsTicking() const { return m_ShouldTick; }

		void DrawDebugLine(const Vector& Start, const Vector& End, const Vector& Color, float Thickness, float Duration);
		void DrawDebugCircle(const Vector& Base, const Vector& X, const Vector& Z, const Vector& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugSphere(const Vector& Center, const Quat& Rotation, const Vector& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugBox(const Box& InBox, const Transform& T, const Vector& Color, float Thickness, float Lifetime);

		inline Scene* GetScene() { return m_Scene; }
		inline PhysicScene* GetPhysicScene() { return m_PhysicScene; }

		inline bool IsPendingDestroy() const { return m_PendingDestory; }

		class LineBatchComponent* m_LineBatchCompponent;

#if WITH_EDITOR

		void Save();

		uint32 GetNonTransientActorCount();

		std::string m_WorldLabel = "UntitledMap";
#endif

	protected:

		void DestroyInternal();
		void DestroyWorldActors();

		std::set<Actor*> m_Actors;

		// actors added in middle of frame get added to actor list at start of next frame
		std::set<Actor*> m_NewActors;

		bool m_ShouldTick;

		std::vector<OnNewActors> OnNewActorsDelegates;
		std::vector<OnRemoveActor> OnRemoveActorDelegates;

		std::string m_LevelPath;
		bool m_Transient;

		Scene* m_Scene;
		PhysicScene* m_PhysicScene;

#if WITH_EDITOR
		StaticMeshActor* m_AxisGridPlane;
#endif

		bool m_PendingDestory;

		friend Scene;
		friend class Level;
		friend class WorldManager;

	private:
	};
}