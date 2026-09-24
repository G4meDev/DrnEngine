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

		bool FindPathPortals(uint32 StartPlane, uint32 EndPlane, std::vector<uint32>& PathProtals);
		void FunnelPath(const Vector& Start, const Vector& End, std::vector<Vector>& PathPoints, const std::vector<uint32>& PathPortals);
		bool FindPath(const Vector& Start, const Vector& End, float AgentRadius, float AgentHeight, std::vector<Vector>& PathPoints, std::vector<uint32>& PathPortals);

		NavMeshConvexHalfEdge ConvexMesh;
		DynamicMeshComponent* Visualizer;

		// TODO: stringid
		std::string Name;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;
		void DrawInternal();
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