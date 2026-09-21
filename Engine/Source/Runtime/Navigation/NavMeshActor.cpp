#include "DrnPCH.h"
#include "NavMeshActor.h"
#include "Runtime/Engine/DynamicMeshComponent.h"

namespace Drn
{
	class NavMeshVisualizer : public DynamicMeshComponent
	{
	public:
		NavMeshVisualizer() : DynamicMeshComponent() {};
		virtual ~NavMeshVisualizer() {};

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override
		{
			DynamicMeshComponent::DrawDetailPanel(DeltaTime);

			NavMesh->DrawInternal();
		}

		virtual Transform GetGizmoTransform() const override
		{
			return NavMesh->GetGizmoTransformVisualizer();
		}

		virtual void OnGizmoTransformChanged(const Transform& GizmoTransform, EGizmoSpace Space) override
		{
			NavMesh->OnGizmoTransformChangedVisualizer(GizmoTransform, Space);
		};

		virtual void SetSelectedInEditor(bool SelectedInEditor, const HitProxyData& Data) override
		{
			DynamicMeshComponent::SetSelectedInEditor(SelectedInEditor, Data);

			NavMesh->SetSelectedInEditorVisualizer(SelectedInEditor, Data);
		}
#endif

		NavMeshComponent* NavMesh;
	};

	NavMeshActor::NavMeshActor()
		: Actor()
	{
		m_NavMeshComponent = std::make_unique<NavMeshComponent>();
		SetRootComponent(m_NavMeshComponent.get());
		m_NavMeshComponent->SetComponentLabel("NavMeshComponent");

		NavMeshVisualizer* Vis = new NavMeshVisualizer;
		m_DynamicMeshComponent.reset(Vis);

		m_NavMeshComponent->AttachSceneComponent(m_DynamicMeshComponent.get());
		m_DynamicMeshComponent->SetComponentLabel("Visualizer");

		m_NavMeshComponent->Visualizer = m_DynamicMeshComponent.get();
		Vis->NavMesh = m_NavMeshComponent.get();

		AssetHandle<StaticMesh> SphereMesh("Engine\\Content\\BasicShapes\\SM_Sphere.drn");
		SphereMesh.Load();

		NavTestStart = std::make_unique<StaticMeshComponent>();
		GetRoot()->AttachSceneComponent(NavTestStart.get());
		NavTestStart->SetComponentLabel("Start");
		NavTestStart->SetMesh(SphereMesh);

		NavTestEnd = std::make_unique<StaticMeshComponent>();
		GetRoot()->AttachSceneComponent(NavTestEnd.get());
		NavTestEnd->SetComponentLabel("End");
		NavTestEnd->SetMesh(SphereMesh);
	}

	NavMeshActor::~NavMeshActor()
	{
		
	}

	void NavMeshActor::Serialize( Archive& Ar )
	{
		Actor::Serialize(Ar);

		m_NavMeshComponent->Serialize(Ar);
		//m_DynamicMeshComponent->Serialize(Ar);
	}

	bool NavMeshActor::DrawDetailPanel()
	{
		Actor::DrawDetailPanel();

		if (ImGui::Button("Test"))
		{
			GetWorld()->FlushDebugLines();

			NavMeshComponent* TestNavMesh = GetWorld()->GetNavigationSystem()->GetNavMesh("");
			NavMeshConvexHalfEdge& Conv = TestNavMesh->ConvexMesh;

			uint32 PlaneIndex = Conv.FindPointsPlane(NavTestStart->GetRelativeLocation());

			if (PlaneIndex != TestNavMesh->ConvexMesh.InvalidIndex)
			{
				const Vector P0 = Conv.GetVertex(Conv.GetPlaneVertex(PlaneIndex, 0)).Position;
				const Vector P1 = Conv.GetVertex(Conv.GetPlaneVertex(PlaneIndex, 1)).Position;
				const Vector P2 = Conv.GetVertex(Conv.GetPlaneVertex(PlaneIndex, 2)).Position;

				GetWorld()->DrawDebugSphere( (P0 + P1 + P2) / 3.0f, Quat::Identity, Color::Green, 0.3f, 32, 0.0f, 100.0f);
			}
		}

		return false;
	}

        }  // namespace Drn