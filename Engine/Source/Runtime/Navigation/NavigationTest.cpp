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

		NavTestStart = std::make_unique<StaticMeshComponent>();
		GetRoot()->AttachSceneComponent(NavTestStart.get());
		NavTestStart->SetComponentLabel("Start");
		NavTestStart->SetMesh(ArrowMesh);

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

			//const Vector& StartRelative = CompTransform.InverseTransformPosition(NavTestStart->GetWorldLocation());
			//const Vector& EndRelative = CompTransform.InverseTransformPosition(NavTestEnd->GetWorldLocation());
			//
			//bool bOverlaps;
			//Vector Nearest;
			//uint32 PlaneIndex = Conv.FindNearestPlane(StartRelative, Nearest, 0.5f, 4, bOverlaps);
			//
			//if (PlaneIndex != TestNavMesh->ConvexMesh.InvalidIndex)
			//{
			//	GetWorld()->DrawDebugSphere(CompTransform.TransformPosition(Nearest) , Quat::Identity, bOverlaps ? Color::Green : Color::Red, 0.3f, 32, 0.0f, 100.0f);
			//}

			std::vector<Vector> PathPoints;
			std::vector<uint32> PathPlanes;

			bool bFoundPath = TestNavMesh->FindPath(NavTestStart->GetWorldLocation(), NavTestEnd->GetWorldLocation(), 0.5f, 4.0f, PathPoints, PathPlanes);
			if (bFoundPath)
			{
				//for (uint32 PlaneIndex : PathPlanes)
				//{
				//	const Vector& PlaneCenter = Conv.GetPlane(PlaneIndex).Center;
				//	GetWorld()->DrawDebugSphere(CompTransform.TransformPosition(PlaneCenter), Quat::Identity, Color::Green, 0.3f, 32, 0.0f, 100.0f);
				//}

				for (uint32 PointIndex = 0; PointIndex < PathPoints.size() - 1; PointIndex++)
				{
					GetWorld()->DrawDebugArrow(PathPoints[PointIndex], PathPoints[PointIndex+1], 0.3f, Color::Green, 0.0f, 100.0f);
				}
			}
		}

		return false;
	}
#endif
}  // namespace Drn