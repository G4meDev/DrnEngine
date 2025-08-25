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

		void InitPlay();
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

		inline double GetTimeSeconds() const { return m_TimeSeconds; }

		void DrawDebugLine(const Vector& Start, const Vector& End, const Color& Color, float Thickness, float Duration);
		void DrawDebugCircle(const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugSphere(const Vector& Center, const Quat& Rotation, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugBox(const Box& InBox, const Transform& T, const Color& Color, float Thickness, float Lifetime);
		void DrawDebugCone(const Vector& InCenter, const Vector& Direction, float Length, float AngleWidth, float AngleHeight, const Color& Color, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugConeCap(const Vector& InCenter, const Vector& Direction, float Length, float Angle, const Color& Color, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugArrow(const Vector& LineStart, const Vector& LineEnd, float ArrowSize, const Color& Color, float Thickness, float Lifetime);
		void DrawDebugFrustum(const Matrix& Frustum, const Color& Color, float Thickness, float Lifetime);

		inline Scene* GetScene() { return m_Scene; }
		inline PhysicScene* GetPhysicScene() { return m_PhysicScene; }

		inline bool IsPendingDestroy() const { return m_PendingDestory; }

		class LineBatchComponent* m_LineBatchCompponent;
		class LineBatchComponent* m_LineBatchThicknessCompponent;

#if WITH_EDITOR
		void Save();

		uint32 GetNonTransientActorCount();

		std::string m_WorldLabel = "UntitledMap";

		void SetLabel( const std::string& Label ) { m_WorldLabel = Label; }
		const std::string& GetLabel() const { return m_WorldLabel; }
#endif

	protected:

		void DestroyInternal();
		void DestroyWorldActors();

		void InitPlayerPawn();
		template<typename T>
		T* GetActorFromClass();

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
		double m_TimeSeconds;

		friend Scene;
		friend class Level;
		friend class WorldManager;

	private:
	};

	template<typename T>
	T* World::GetActorFromClass()
	{
		for (Actor* A : m_Actors)
		{
			if (A->GetActorType() == T::GetActorTypeStatic())
				return static_cast<T*>(A);
		}

		for (Actor* A : m_NewActors)
		{
			if (A->GetActorType() == T::GetActorTypeStatic())
				return static_cast<T*>(A);
		}

		return nullptr;
	}

}