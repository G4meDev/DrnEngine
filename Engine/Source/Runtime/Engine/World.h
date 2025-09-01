#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogWorld);

namespace Drn
{
	DECLARE_MULTICAST_DELEGATE_OneParam( OnAddActorsDelegate, const std::set<Actor*>&);
	DECLARE_MULTICAST_DELEGATE_OneParam( OnRemoveActorsDelegate, const std::vector<Actor*>&);

	enum class EWorldViewFlag : uint64
	{
		None				= 0,
		Collision			= 1 << 0,
		Light				= 1 << 1,
	};

	enum class EWorldType
	{
		Editor,
		PlayInEditor,
		Game,
	};

	class World
	{
	public:
		World();
		~World();

		void Destroy();

		void InitPlay();
		void Tick(float DeltaTime);

		template<typename T>
		T* SpawnActor()
		{
			T* NewActor = new T();
			NewActor->m_World = this;
			m_NewActors.insert(NewActor);

			return NewActor;
		}

		Component* GetComponentWithGuid(const Guid& ID);

		inline const std::set<Actor*>& GetActorList() { return m_Actors; };

		OnAddActorsDelegate OnAddActors;
		OnRemoveActorsDelegate OnRemoveActors;

		inline void SetTransient( bool Transient ) { m_Transient = Transient; }
		inline bool IsTransient() { return m_Transient; }

		inline class PlayerController* GetPlayerController() const { return m_PlayerController; }

		inline double GetTimeSeconds() const { return m_TimeSeconds; }

		void DrawDebugLine(const Vector& Start, const Vector& End, const Color& Color, float Thickness, float Duration);
		void DrawDebugCircle(const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugSphere(const Vector& Center, const Quat& Rotation, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugBox(const Box& InBox, const Transform& T, const Color& Color, float Thickness, float Lifetime);
		void DrawDebugCone(const Vector& InCenter, const Vector& Direction, float Length, float AngleWidth, float AngleHeight, const Color& Color, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugConeCap(const Vector& InCenter, const Vector& Direction, float Length, float Angle, const Color& Color, int32 NumSides, float Thickness, float Lifetime);
		void DrawDebugArrow(const Vector& LineStart, const Vector& LineEnd, float ArrowSize, const Color& Color, float Thickness, float Lifetime);
		void DrawDebugFrustum(const Matrix& Frustum, const Color& Color, float Thickness, float Lifetime);

		void DrawDebugCapsule(const Vector& Center, float HalfHeight, float Radius, const Quat& Rotation, const Color& Color, float Thickness, float Lifetime);

		inline Scene* GetScene() { return m_Scene; }
		inline PhysicScene* GetPhysicScene() { return m_PhysicScene; }

		inline bool IsPendingDestroy() const { return m_PendingDestory; }

		ViewInfo GetPlayerWorldView() const;

		inline void SetEditorWorld() { m_WorldType = EWorldType::Editor; }
		inline void SetPlayInEditorWorld() { m_WorldType = EWorldType::PlayInEditor; }
		inline void SetGameWorld() { m_WorldType = EWorldType::Game; }
		inline void SetPaused(bool Paused) { m_Paused = Paused; }

		inline bool IsEditorWorld() const { return m_WorldType == EWorldType::Editor; }
		inline bool IsPlayInEditorWorld() const { return m_WorldType == EWorldType::PlayInEditor; }
		inline bool IsGameWorld() const { return m_WorldType == EWorldType::Game; }

		inline bool IsPaused() const { return m_Paused; }

		class LineBatchComponent* m_LineBatchCompponent;
		class LineBatchComponent* m_LineBatchThicknessCompponent;

#if WITH_EDITOR

		EWorldViewFlag m_WorldViewFlags = EWorldViewFlag::None;

		inline bool HasViewFlag( EWorldViewFlag Flag ) const { return static_cast<uint64>(m_WorldViewFlags) & static_cast<uint64>(Flag); }
		inline void SetViewFlag( EWorldViewFlag Flag, bool Value )
		{
			if (Value)
			{
				m_WorldViewFlags = static_cast<EWorldViewFlag>(static_cast<uint64>(m_WorldViewFlags) | static_cast<uint64>(Flag));
			}

			else
			{
				m_WorldViewFlags = static_cast<EWorldViewFlag>(static_cast<uint64>(m_WorldViewFlags) & ~static_cast<uint64>(Flag));
			}
		}

		void SetEjected(bool Ejected);
		inline bool IsEjected() const { return m_Ejected; }
		inline bool ShouldUseViewportCamera() const { return m_WorldType == EWorldType::Editor || IsEjected(); }
		inline CameraActor* GetViewportCamera() const { return m_ViewportCamera; }

		void Save();

		uint32 GetNonTransientActorCount();

		std::string m_WorldLabel = "UntitledMap";

		void SetLabel( const std::string& Label ) { m_WorldLabel = Label; }
		const std::string& GetLabel() const { return m_WorldLabel; }
#endif

	protected:

		void DestroyActor(Actor* InActor);

		void DestroyInternal();
		void DestroyWorldActorsAndComponents();

		void InitPlayerPawn();
		template<typename T>
		T* GetActorFromClass();

		std::set<Actor*> m_Actors;

		// actors added in middle of frame get added to actor list at start of next frame
		std::set<Actor*> m_NewActors;

		std::vector<Component*> m_PendingKillComponents;

		inline void AddPendingkillComponent( Component* InComponent )
		{
			if (InComponent)
				m_PendingKillComponents.push_back(InComponent);
		};

		std::string m_LevelPath;
		bool m_Transient;

		Scene* m_Scene;
		PhysicScene* m_PhysicScene;

		class PlayerController* m_PlayerController;

#if WITH_EDITOR
		StaticMeshActor* m_AxisGridPlane;
#endif

		bool m_PendingDestory;
		float m_TimeSeconds;
		float m_UnpausedSeconds;

		bool m_Paused;

		EWorldType m_WorldType;

#if WITH_EDITOR
		bool m_Ejected = false;
		bool m_EverEjected = false;
		class CameraActor* m_ViewportCamera = nullptr;
#endif

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