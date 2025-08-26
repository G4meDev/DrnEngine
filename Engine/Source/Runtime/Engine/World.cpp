#include "DrnPCH.h"
#include "World.h"

#include "Runtime/Physic/PhysicManager.h"
#include "Runtime/Components/LineBatchComponent.h"

LOG_DEFINE_CATEGORY( LogWorld, "LogWorld" );

namespace Drn
{
	World::World() 
		: m_ShouldTick(false)
		, m_LevelPath("")
		, m_Transient(false)
		, m_PendingDestory(false)
		, m_TimeSeconds(0)
	{
		m_PhysicScene = PhysicManager::Get()->AllocateScene(this);
		m_Scene = Renderer::Get()->AllocateScene(this);

		// TODO: proper camera and scene render management
#ifndef WITH_EDITOR
		CameraActor* Cam = SpawnActor<CameraActor>();
		Cam->SetActorLocation(XMVectorSet(0, 5, -10, 0));

		Renderer::Get()->m_MainSceneRenderer = m_Scene->AllocateSceneRenderer();
		Renderer::Get()->m_MainSceneRenderer->m_CameraActor = Cam;
#endif

#if WITH_EDITOR

		AssetHandle<StaticMesh> AxisGridMesh( "Engine\\Content\\BasicShapes\\SM_Plane.drn" );
		AxisGridMesh.Load();
		AssetHandle<Material> AxisGridMaterial( "Engine\\Content\\Materials\\M_AxisGridMaterial.drn" );
		AxisGridMaterial.Load();
		
		m_AxisGridPlane = SpawnActor<StaticMeshActor>();
		m_AxisGridPlane->SetActorLabel("AxisGrid");
		m_AxisGridPlane->SetTransient(true);
		m_AxisGridPlane->GetMeshComponent()->SetSelectable(false);
		m_AxisGridPlane->SetActorScale(Vector( 1000.0f ));
		m_AxisGridPlane->SetActorLocation(Vector::ZeroVector);
		m_AxisGridPlane->GetMeshComponent()->SetMesh(AxisGridMesh);
		m_AxisGridPlane->GetMeshComponent()->SetMaterial(0, AxisGridMaterial);
		m_AxisGridPlane->GetMeshComponent()->SetEditorPrimitive(true);

#endif

		m_LineBatchCompponent = new LineBatchComponent();
		m_LineBatchCompponent->SetThickness(false);
		m_LineBatchCompponent->RegisterComponent(this);

		m_LineBatchThicknessCompponent = new class LineBatchComponent();
		m_LineBatchThicknessCompponent->SetThickness(true);
		m_LineBatchThicknessCompponent->RegisterComponent(this);
	}

	World::~World()
	{
		
	}

	void World::Destroy()
	{
		m_PendingDestory = true;
	}

	void World::InitPlay()
	{
		InitPlayerPawn();
	}

	void World::Tick( float DeltaTime )
	{
		SCOPE_STAT();

		m_TimeSeconds += DeltaTime;

		{
			SCOPE_STAT("PendingDestroyAddActors");

			for (Component* Comp : m_PendingKillComponents)
			{
				delete Comp;
			}
			m_PendingKillComponents.clear();

			for (auto it = m_Actors.begin(); it != m_Actors.end();)
			{
				if (*it && (*it)->IsMarkedPendingKill())
				{
					Actor* ToDelActor = *it;
					it = m_Actors.erase(it);
					DestroyActor(ToDelActor);
				}

				else
				{
					it++;
				}
			}
		
			for (auto it = m_NewActors.begin(); it != m_NewActors.end();)
			{
				if (*it && (*it)->IsMarkedPendingKill())
				{
					Actor* ToDelActor = *it;
					it = m_NewActors.erase(it);
					DestroyActor(ToDelActor);
				}
				else
				{
					(*it)->RegisterComponents(this);

					it++;
				}
			}

			if (m_NewActors.size())
			{
				m_Actors.insert(m_NewActors.begin(), m_NewActors.end());

				OnAddActors.Braodcast(m_NewActors);
				m_NewActors.clear();
			}
		}

		m_LineBatchCompponent->TickComponent(DeltaTime);
		m_LineBatchThicknessCompponent->TickComponent(DeltaTime);

		if (!m_ShouldTick)
		{
			return;
		}

		{
			SCOPE_STAT();

			for (Actor* actor : m_Actors)
			{
				actor->Tick(DeltaTime);
			}
		}
	}

	Component* World::GetComponentWithGuid( const Guid& ID )
	{
		if (ID.IsValid())
		{
			for (Actor* actor : m_Actors)
			{
				if (!actor->IsMarkedPendingKill())
				{
					std::vector<Component*> Components;
					actor->GetComponents<Component>(Components, EComponentType::Component, true);

					for (Component* Comp : Components)
					{
						if (Comp->GetGuid() == ID)
						{
							return Comp;
						}
					}
				}
			}
		}

		return nullptr;
	}

// ----------------------------------------------------------------------------------------

	void World::DrawDebugLine( const Vector& Start, const Vector& End, const Color& Color, float Thickness, float Duration )
	{
		if (m_LineBatchCompponent && Thickness == 0)
		{
			m_LineBatchCompponent->DrawLine(Start, End, Color, Thickness, Duration);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawLine(Start, End, Color, Thickness, Duration);
		}
	}

	void World::DrawDebugCircle( const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		if (m_LineBatchCompponent && Thickness == 0)
		{
			m_LineBatchCompponent->DrawCircle(Base, X, Z, Color, Radius, NumSides, Thickness, Lifetime);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawCircle(Base, X, Z, Color, Radius, NumSides, Thickness, Lifetime);
		}
	}

	void World::DrawDebugSphere( const Vector& Center, const Quat& Rotation, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		if (m_LineBatchCompponent && Thickness == 0)
		{
			m_LineBatchCompponent->DrawSphere(Center, Rotation, Color, Radius, NumSides, Thickness, Lifetime);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawSphere(Center, Rotation, Color, Radius, NumSides, Thickness, Lifetime);
		}
	}

	void World::DrawDebugBox( const Box& InBox, const Transform& T, const Color& Color, float Thickness, float Lifetime )
	{
		if (m_LineBatchCompponent)
		{
			m_LineBatchCompponent->DrawBox(InBox, T, Color, Thickness, Lifetime);
		}
		else if(m_LineBatchThicknessCompponent && Thickness != 0)
		{
			m_LineBatchThicknessCompponent->DrawBox(InBox, T, Color, Thickness, Lifetime);
		}
	}

	void World::DrawDebugCone( const Vector& InCenter, const Vector& Direction, float Length, float AngleWidth,
		float AngleHeight, const Color& Color, int32 NumSides, float Thickness, float Lifetime )
	{
		NumSides = std::max<uint32>(NumSides, 4);

		const float Angle1 = std::clamp<float>(AngleHeight, (float)KINDA_SMALL_NUMBER, (float)(Math::PI - KINDA_SMALL_NUMBER));
		const float Angle2 = std::clamp<float>(AngleWidth, (float)KINDA_SMALL_NUMBER, (float)(Math::PI - KINDA_SMALL_NUMBER));

		const float SinX_2 = std::sin(0.5f * Angle1);
		const float SinY_2 = std::sin(0.5f * Angle2);

		const float SinSqX_2 = SinX_2 * SinX_2;
		const float SinSqY_2 = SinY_2 * SinY_2;

		std::vector<Vector> ConeVerts;
		ConeVerts.resize(NumSides);

		for(int32 i = 0; i < NumSides; i++)
		{
			const float Fraction	= (float)i/(float)(NumSides);
			const float Thi			= 2.f * Math::PI * Fraction;
			const float Phi			= std::atan2(std::sin(Thi)*SinY_2, std::cos(Thi)*SinX_2);
			const float SinPhi		= std::sin(Phi);
			const float CosPhi		= std::cos(Phi);
			const float SinSqPhi	= SinPhi*SinPhi;
			const float CosSqPhi	= CosPhi*CosPhi;

			const float RSq			= SinSqX_2*SinSqY_2 / (SinSqX_2*SinSqPhi + SinSqY_2*CosSqPhi);
			const float R			= std::sqrt(RSq);
			const float Sqr			= std::sqrt(1-RSq);
			const float Alpha		= R*CosPhi;
			const float Beta		= R*SinPhi;

			ConeVerts[i] = Vector(1 - 2 * RSq, 2 * Sqr * Alpha, 2 * Sqr * Beta);
		}

		Vector YAxis, ZAxis;

		Vector DirectionNorm = Direction.GetSafeNormal();
		DirectionNorm.FindBestAxisVectors(YAxis, ZAxis);

		const Matrix ConeToWorld = Matrix::ScaleMatrix(Vector(Length)) * Matrix(DirectionNorm, YAxis, ZAxis, InCenter);
		Transform ConeToWorldTransform = Transform(ConeToWorld);

		Vector CurrentPoint, PrevPoint, FirstPoint;
		for(int32 i = 0; i < NumSides; i++)
		{
			CurrentPoint = ConeToWorldTransform.TransformPosition(ConeVerts[i]);
			DrawDebugLine(ConeToWorldTransform.GetLocation(), CurrentPoint, Color, Lifetime, Thickness);

			if( i > 0 )
			{
				DrawDebugLine(PrevPoint, CurrentPoint, Color, Lifetime, Thickness);
			}
			else
			{
				FirstPoint = CurrentPoint;
			}

			PrevPoint = CurrentPoint;
		}
		DrawDebugLine(CurrentPoint, FirstPoint, Color, Lifetime, Thickness);
	}

	void World::DrawDebugConeCap( const Vector& InCenter, const Vector& Direction, float Length, float Angle,
		const Color& Color, int32 NumSides, float Thickness, float Lifetime )
	{
		Vector YAxis, XAxis;

		Vector DirectionNorm = Direction.GetSafeNormal();
		DirectionNorm.FindBestAxisVectors(YAxis, XAxis);

		const Matrix ConeToWorld = Matrix::ScaleMatrix(Vector(Length)) * Matrix(XAxis, YAxis, DirectionNorm, InCenter);
		Transform ConeToWorldTransform = Transform(ConeToWorld);

		const float StepSize = Angle * 2 / NumSides;
		Vector PrevPosition_1 = Vector( std::sin(-Angle), 0, std::cos( -Angle ));
		Vector PrevPosition_2 = ConeToWorldTransform.TransformPosition(Quat(XM_PIDIV2, 0, 0).RotateVector(PrevPosition_1));
		PrevPosition_1 = ConeToWorldTransform.TransformPosition(PrevPosition_1);

		for (int i = 1; i <= NumSides; i++)
		{
			const float A = -Angle + StepSize * i;
			Vector Position_1 = Vector( std::sin(A), 0, std::cos( A ));
			Vector Position_2 = ConeToWorldTransform.TransformPosition(Quat(XM_PIDIV2, 0, 0).RotateVector(Position_1));
			Position_1 = ConeToWorldTransform.TransformPosition(Position_1);

			DrawDebugLine(PrevPosition_1, Position_1, Color::White, 0, 0);
			DrawDebugLine(PrevPosition_2, Position_2, Color::White, 0, 0);
			PrevPosition_1 = Position_1;
			PrevPosition_2 = Position_2;
		}
	}

	void World::DrawDebugArrow( const Vector& LineStart, const Vector& LineEnd, float ArrowSize, const Color& Color, float Thickness, float Lifetime )
	{
		DrawDebugLine(LineStart, LineEnd, Color, Thickness, Lifetime);

		Vector Direction = LineEnd - LineStart;
		Direction = Direction.GetSafeNormal();
		Vector Up = Vector::UpVector;
		Vector Right = Direction ^ Up;

		if (!Right.IsNormalized())
		{
			Direction.FindBestAxisVectors(Up, Right);
		}

		Matrix TM(Direction, Right, Up, Vector::ZeroVector);
		Transform T(TM);

		float SizeSqrt = std::sqrt(ArrowSize);
		DrawDebugLine(LineEnd, LineEnd + T.TransformPosition(Vector(-SizeSqrt, 0, SizeSqrt)), Color, Thickness, Lifetime);
		DrawDebugLine(LineEnd, LineEnd + T.TransformPosition(Vector(-SizeSqrt, 0, -SizeSqrt)), Color, Thickness, Lifetime);
	}

	void World::DrawDebugFrustum( const Matrix& Frustum, const Color& Color, float Thickness, float Lifetime ) 
	{
		Vector Vertices[2][2][2];

		for (uint32 Z = 0; Z < 2; Z++)
		{
			for (uint32 Y = 0; Y < 2; Y++)
			{
				for (uint32 X = 0; X < 2; X++)
				{
					Vector4 UnprojetcedVertex = Frustum.TransformVector4(Vector4(X ? -1 : 1, Y ? -1 : 1, Z ? 0 : 1, 1));
					Vertices[X][Y][Z] = Vector(UnprojetcedVertex.GetX(), UnprojetcedVertex.GetY(), UnprojetcedVertex.GetZ()) / UnprojetcedVertex.GetW();
				}
			}
		}

		DrawDebugLine(Vertices[0][0][0], Vertices[0][0][1], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[1][0][0], Vertices[1][0][1], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[0][1][0], Vertices[0][1][1], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[1][1][0], Vertices[1][1][1], Color, Thickness, Lifetime);

		DrawDebugLine(Vertices[0][0][0], Vertices[0][1][0], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[1][0][0], Vertices[1][1][0], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[0][0][1], Vertices[0][1][1], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[1][0][1], Vertices[1][1][1], Color, Thickness, Lifetime);

		DrawDebugLine(Vertices[0][0][0], Vertices[1][0][0], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[0][1][0], Vertices[1][1][0], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[0][0][1], Vertices[1][0][1], Color, Thickness, Lifetime);
		DrawDebugLine(Vertices[0][1][1], Vertices[1][1][1], Color, Thickness, Lifetime);
	}

	void World::DestroyActor( Actor* InActor )
	{
		std::vector<Actor*> RemovedActorList;
		RemovedActorList.push_back(InActor);
		OnRemoveActors.Braodcast( RemovedActorList );

		for (Component* Comp : InActor->Components)
		{
			Comp->DestroyComponent();
		}

		// TODO: destroy recursive on scene comp
		//for (Component* Comp : InActor->Components)
		//{
		//	Comp->DestroyComponent();
		//}

		//InActor->UnRegisterComponents();
		delete InActor;
	}

	void World::DestroyInternal()
	{
		m_LineBatchCompponent->UnRegisterComponent();
		delete m_LineBatchCompponent;
		m_LineBatchCompponent = nullptr;

		DestroyWorldActorsAndComponents();

		PhysicManager::Get()->RemoveAndInvalidateScene(m_PhysicScene);
		Renderer::Get()->ReleaseScene(m_Scene);

		delete this;
	}

	void World::DestroyWorldActorsAndComponents()
	{
		for (auto it = m_Actors.begin(); it != m_Actors.end(); )
		{
			Actor* ToDelActor = *it;
			it = m_Actors.erase(it);
			DestroyActor(ToDelActor);
		}
		
		for (auto it = m_NewActors.begin(); it != m_NewActors.end(); )
		{
			Actor* ToDelActor = *it;
			it = m_NewActors.erase(it);
			DestroyActor(ToDelActor);
		}

		for (Component* Comp : m_PendingKillComponents)
		{
			delete Comp;
		}
	}

	void World::InitPlayerPawn()
	{
		Controller* PC = SpawnActor<Controller>();
		PC->SetActorLabel("PlayerController");

		Pawn* PlayerPawn = nullptr;
		for (Actor* A : m_NewActors)
		{
			// TODO: actor and component type inheritance e.g. Character -> Pawn -> Actor
			if (A->GetActorType() == EActorType::Pawn)
			{
				Pawn* P = static_cast<Pawn*>(A);
				if (P->GetAutoPossessPlayer())
				{
					PlayerPawn = P;
					break;
				}
			}
		}

		if (!PlayerPawn)
		{
			PlayerPawn = SpawnActor<Pawn>();
			PlayerPawn->SetActorLabel("PlayerPawn");
		}

		PlayerPawn->PossessBy(PC);
	}

#if WITH_EDITOR

	void World::Save()
	{
		if (IsTransient())
		{
			LOG(LogWorld, Warning, "trying to save a transient world.");
			return;
		}

		AssetHandle<Level> LevelHandle(m_LevelPath);
		LevelHandle.Load();

		LevelHandle->SaveFromWorld(this);
	}

	uint32 World::GetNonTransientActorCount()
	{
		int result = 0;

		for (Actor* Actor : m_Actors)
		{
			if (!Actor->IsTransient())
			{
				result++;
			}
		}

		return result;
	}

#endif

}