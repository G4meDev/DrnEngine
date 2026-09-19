#pragma once

#include "ForwardTypes.h"
#include "Runtime/Navigation/NavMeshConvexHalfEdge.h"

namespace Drn
{
	enum class ENavMeshElementType : uint32
	{
		Invalid = 0,
		Face,
		Vertex,
		Edge
	};

	class NavMeshComponent : public SceneComponent
	{
	public:
		NavMeshComponent();
		virtual ~NavMeshComponent();

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform( bool SkipPhysic ) override;

		virtual EComponentType GetComponentType() override { return EComponentType::NavMeshComponent; }

		void UpdateVisualizer();
		void UpdateVisualizerFaces();
		void UpdateVisualizerVertecies();
		void UpdateVisualizerEdge();

		NavMeshConvexHalfEdge ConvexMesh;
		DynamicMeshComponent* Visualizer;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		inline virtual bool HasSprite() const override { return true; }
		virtual void SetSelectedInEditor( bool SelectedInEditor, const HitProxyData& Data ) override;

		Transform GetGizmoTransformVisualizer() const;
		void OnGizmoTransformChangedVisualizer(const Transform& GizmoTransform, EGizmoSpace Space);
		void SetSelectedInEditorVisualizer(bool SelectedInEditor, const HitProxyData& Data);

		ENavMeshElementType SelectedElementType = ENavMeshElementType::Invalid;
		uint32 SelectedElement = 0;
#endif
	};
}