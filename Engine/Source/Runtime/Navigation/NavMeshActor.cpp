#include "DrnPCH.h"
#include "NavMeshActor.h"
#include "Runtime/Engine/DynamicMeshComponent.h"

namespace Drn
{
	class NavMeshVisualizer : public DynamicMeshComponent
	{
	public:
		NavMeshVisualizer() : DynamicMeshComponent()
		{
			SetEditorPrimitive(true);
		};

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
        }  // namespace Drn