#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Delegate.h"

LOG_DECLARE_CATEGORY(LogWorld);

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnAddActorsDelegate, const std::set<Actor*>&);
	DECLARE_MULTICAST_DELEGATE_OneParam( OnRemoveActorsDelegate, const std::vector<Actor*>&);

	class World
	{
	public:
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

		OnAddActorsDelegate OnAddActors;
		OnRemoveActorsDelegate OnRemoveActors;

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
		class LineBatchComponent* m_LineBatchThicknessCompponent;

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