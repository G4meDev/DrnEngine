#include "DrnPCH.h"
#include "NavigationTest.h"

namespace Drn
{
	NavigationTest::NavigationTest() : Actor()
	{
		Root = std::make_unique<SceneComponent>();
		SetRootComponent(Root.get());
		Root->SetComponentLabel("Root");

		AssetHandle<StaticMesh> ArrowMesh("Engine\\Content\\Template\\Common\\Mesh\\SM_DestinationArrow.drn");
		ArrowMesh.Load();

		AssetHandle<MaterialInstance> Mat("Engine\\Content\\Template\\Common\\Material\\MI_SolidColorUnlit_C.drn");
		Mat.Load();

		NavTestStart = std::make_unique<StaticMeshComponent>();
		GetRoot()->AttachSceneComponent(NavTestStart.get());
		NavTestStart->SetComponentLabel("Start");
		NavTestStart->SetMesh(ArrowMesh);
		NavTestStart->SetMaterial(0, Mat);

		NavTestEnd = std::make_unique<StaticMeshComponent>();
		GetRoot()->AttachSceneComponent(NavTestEnd.get());
		NavTestEnd->SetComponentLabel("End");
		NavTestEnd->SetMesh(ArrowMesh);
		NavTestEnd->SetRelativeLocation(Vector::ForwardVector * 5);
	}

	NavigationTest::~NavigationTest()
	{
		
	}

	void NavigationTest::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		Root->Serialize(Ar);
		NavTestStart->Serialize(Ar);
		NavTestEnd->Serialize(Ar);
	}

	void NavigationTest::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);

		if (!TestCharacter)
		{
			TestCharacter = GetWorld()->SpawnActor<ThirdPersonCharacter>();
			Randomize();
			TestCharacter->SetActorLocation(NavTestStart->GetWorldLocation());
			TestCharacter->OnBeginRun();
		}

		const Vector CharacterLocation = TestCharacter->GetActorLocation();
		const Vector TargetLocation = NavTestEnd->GetWorldLocation();
		const bool bReached = CharacterLocation.IsNearlyEqual(TargetLocation, 1.5f);
		if (bReached)
		{
			Randomize();
		}
		else
		{
			NavMeshComponent* TestNavMesh = GetWorld()->GetNavigationSystem()->GetNavMesh("");
		
			std::vector<Vector> PathPoints;
			bool bFoundPath = TestNavMesh->FindPath(CharacterLocation, TargetLocation, 0.5f, 4.0f, PathPoints);

			if (bFoundPath)
			{
				drn_check(PathPoints.size() > 1);
				const Vector MovementDirection = (PathPoints[1] - PathPoints[0]).GetSafeNormal();
				TestCharacter->SetMovementInputToWorldDirection(MovementDirection);

				for (uint32 PointIndex = 0; (PathPoints.size() >= 2) && PointIndex < (PathPoints.size()-1); PointIndex++)
				{
					GetWorld()->DrawDebugArrow(PathPoints[PointIndex] + Vector(0, 2, 0), PathPoints[PointIndex+1] + Vector(0, 2, 0), 0.3f, Color::Blue, 0.0f, 0.0f);
				}
			}
		}
	}

#if WITH_EDITOR
	bool NavigationTest::DrawDetailPanel()
	{
		Actor::DrawDetailPanel();

		if (ImGui::Button("Test"))
		{
			GetWorld()->FlushDebugLines();

			NavMeshComponent* TestNavMesh = GetWorld()->GetNavigationSystem()->GetNavMesh("");
			NavMeshConvexHalfEdge& Conv = TestNavMesh->ConvexMesh;
			Transform CompTransform = TestNavMesh->GetWorldTransform();

			std::vector<Vector> PathPoints;
			bool bFoundPath = TestNavMesh->FindPath(NavTestStart->GetWorldLocation(), NavTestEnd->GetWorldLocation(), 0.5f, 4.0f, PathPoints);
			if (bFoundPath)
			{
				for (uint32 PointIndex = 0; (PathPoints.size() >= 2) && PointIndex < (PathPoints.size()-1); PointIndex++)
				{
					GetWorld()->DrawDebugArrow(PathPoints[PointIndex] + Vector(0, 2, 0), PathPoints[PointIndex+1] + Vector(0, 2, 0), 0.3f, Color::Blue, 0.0f, 100.0f);
				}
			}
		}

		if (ImGui::Button("Randomize"))
		{
			Randomize();
		}

		return false;
	}

	void NavigationTest::Randomize()
	{
		NavMeshComponent* TestNavMesh = GetWorld()->GetNavigationSystem()->GetNavMesh("");
		if (TestNavMesh)
		{
			NavTestStart->SetWorldLocation(TestNavMesh->GetRandomPoint());
			NavTestEnd->SetWorldLocation(TestNavMesh->GetRandomPoint());
		}
	}

#endif
}  // namespace Drn