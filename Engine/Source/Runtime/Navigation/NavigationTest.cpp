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

			const Vector& StartRelative = CompTransform.InverseTransformPosition(NavTestStart->GetWorldLocation());
			const Vector& EndRelative = CompTransform.InverseTransformPosition(NavTestEnd->GetWorldLocation());

			uint32 PlaneIndex = Conv.FindPointsPlane(StartRelative);

			if (PlaneIndex != TestNavMesh->ConvexMesh.InvalidIndex)
			{
				const Vector P0 = Conv.GetVertex(Conv.GetPlaneVertex(PlaneIndex, 0)).Position;
				const Vector P1 = Conv.GetVertex(Conv.GetPlaneVertex(PlaneIndex, 1)).Position;
				const Vector P2 = Conv.GetVertex(Conv.GetPlaneVertex(PlaneIndex, 2)).Position;

				const Vector Center = (P0 + P1 + P2) / 3.0f;

				GetWorld()->DrawDebugSphere(CompTransform.TransformPosition(Center) , Quat::Identity, Color::Green, 0.3f, 32, 0.0f, 100.0f);
			}
		}

		return false;
	}
#endif
}  // namespace Drn