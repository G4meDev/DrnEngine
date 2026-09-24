#include "DrnPCH.h"
#include "NavMeshComponent.h"
#include "Runtime/Engine/DynamicMeshComponent.h"

#define VISUALIZER_SLOT_FACE 0
#define VISUALIZER_SLOT_VERTEX 1
#define VISUALIZER_SLOT_EDGE 2

namespace Drn
{
	NavMeshComponent::NavMeshComponent()
		: SceneComponent()
		, Name("")
	{
	}

	NavMeshComponent::~NavMeshComponent()
	{
		
	}

	void NavMeshComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			BufferArchive CompressAr(1000);
			Ar >> CompressAr;
			CompressAr.Decompress();
			CompressAr >> Name;
			CompressAr >> ConvexMesh;

			ConvexMesh.Rebuild();
			UpdateVisualizer();
		}
		else
		{
			BufferArchive CompressAr(1000);
			CompressAr << Name;
			CompressAr << ConvexMesh;
			CompressAr.Compress();

			Ar << CompressAr;
		}
	}

	void NavMeshComponent::RegisterComponent( World* InOwningWorld )
	{
		SceneComponent::RegisterComponent(InOwningWorld);

		GetWorld()->GetNavigationSystem()->RegisterNavMeshComponent(this);

#if WITH_EDITOR
		AssetHandle<Texture2D> DefaultIcon( "Engine\\Content\\EditorResources\\ComponentIcons\\T_NavMeshIcon.drn" );
		DefaultIcon.Load();
		
		m_Sprite->SetSprite( DefaultIcon );
#endif
	}

	void NavMeshComponent::UnRegisterComponent()
	{
		GetWorld()->GetNavigationSystem()->UnregisterNavMeshComponent(this);

		SceneComponent::UnRegisterComponent();
	}

	void NavMeshComponent::OnUpdateTransform( bool SkipPhysic )
	{
		SceneComponent::OnUpdateTransform(SkipPhysic);

	}

	void NavMeshComponent::UpdateVisualizer()
	{
		Visualizer->ClearSections();

		UpdateVisualizerFaces();
		UpdateVisualizerVertecies();
		UpdateVisualizerEdge();
	}

	void NavMeshComponent::UpdateVisualizerFaces()
	{
		AssetHandle<Material> VisualizerMaterial("\\Engine\\Content\\Materials\\M_NavMeshVisalizer_Face.drn");
		VisualizerMaterial.Load();

		std::vector<Vector> Positions;
		std::vector<uint32> Indices;
		std::vector<Color> Colors;

		ConvexMesh.GetIndexedFaces(Positions, Indices);

		Colors.resize(Positions.size(), Color(0, 0, 0, 0));
		if (SelectedElementType == ENavMeshElementType::Face)
		{
			for (uint32 VertexIndex = 0; VertexIndex < 3; VertexIndex++)
			{
				Colors[SelectedElement * 3 + VertexIndex].R = 255;
			}
		}

		Visualizer->CreateMeshSection_Color(VISUALIZER_SLOT_FACE, VisualizerMaterial, Positions, Indices, Colors);
	}

	void NavMeshComponent::UpdateVisualizerVertecies()
	{
		AssetHandle<Material> VisualizerMaterial("\\Engine\\Content\\Materials\\M_NavMeshVisalizer_Vertex.drn");
		VisualizerMaterial.Load();

		std::vector<Vector> Positions;
		std::vector<uint32> Indices;
		std::vector<Color> Colors;

		std::vector<Vector> VerteciesPosition;
		ConvexMesh.GetVeteciesPosition(VerteciesPosition);

		Positions.reserve(VerteciesPosition.size() * 3);
		Indices.reserve(VerteciesPosition.size() * 3);
		for (uint32 VertexIndex = 0; VertexIndex < VerteciesPosition.size(); VertexIndex++)
		{
			const Vector& VertexCenter = VerteciesPosition[VertexIndex];

			const float Scale = 0.15f;

			Positions.push_back(VertexCenter + Vector(-Scale, 0, -Scale/2));
			Positions.push_back(VertexCenter + Vector(0, 0, Scale/2));
			Positions.push_back(VertexCenter + Vector(Scale, 0, -Scale/2));

			Indices.push_back(VertexIndex * 3 + 0);
			Indices.push_back(VertexIndex * 3 + 1);
			Indices.push_back(VertexIndex * 3 + 2);
		}

		Colors.resize(Positions.size(), Color(0, 0, 0, 0));
		if (SelectedElementType == ENavMeshElementType::Vertex)
		{
			for (uint32 VertexIndex = 0; VertexIndex < 3; VertexIndex++)
			{
				Colors[SelectedElement * 3 + VertexIndex].R = 255;
			}
		}

		Visualizer->CreateMeshSection_Color(VISUALIZER_SLOT_VERTEX, VisualizerMaterial, Positions, Indices, Colors);
	}

	void NavMeshComponent::UpdateVisualizerEdge()
	{
		AssetHandle<Material> VisualizerMaterial("\\Engine\\Content\\Materials\\M_NavMeshVisalizer_Edge.drn");
		VisualizerMaterial.Load();

		std::vector<Vector> Positions;
		std::vector<uint32> Indices;
		std::vector<Color> Colors;

		const uint32 EdgeCount = ConvexMesh.NumHalfEdges();
		const int32 VertexCountPerEdge = 6;

		Positions.reserve(EdgeCount * VertexCountPerEdge);
		Indices.reserve(EdgeCount * VertexCountPerEdge);
		for (uint32 EdgeIndex = 0; EdgeIndex < EdgeCount; EdgeIndex++)
		{
			const Vector& EdgePt0 = ConvexMesh.GetVertex(ConvexMesh.GetHalfEdgeVertex(EdgeIndex)).Position;
			const Vector& EdgePt1 = ConvexMesh.GetVertex(ConvexMesh.GetHalfEdgeVertex(ConvexMesh.GetNextHalfEdge(EdgeIndex))).Position;

			const Vector& Dir = (EdgePt1 - EdgePt0).GetSafeNormal();
			const Vector Right = (Dir ^ Vector::UpVector).GetSafeNormal();

			const float Scale = 0.03f;

			const Vector Pt0 = EdgePt0 + Right * Scale;
			const Vector Pt1 = EdgePt0 + Right * -Scale;
			const Vector Pt2 = EdgePt1 + Right * -Scale;
			const Vector Pt3 = EdgePt1 + Right * Scale;

			Positions.push_back(Pt0);
			Positions.push_back(Pt1);
			Positions.push_back(Pt2);
			Positions.push_back(Pt0);
			Positions.push_back(Pt2);
			Positions.push_back(Pt3);

			Indices.push_back(EdgeIndex * VertexCountPerEdge + 0);
			Indices.push_back(EdgeIndex * VertexCountPerEdge + 1);
			Indices.push_back(EdgeIndex * VertexCountPerEdge + 2);
			Indices.push_back(EdgeIndex * VertexCountPerEdge + 3);
			Indices.push_back(EdgeIndex * VertexCountPerEdge + 4);
			Indices.push_back(EdgeIndex * VertexCountPerEdge + 5);
		}

		Colors.resize(Positions.size(), Color(0, 0, 0, 0));
		if (SelectedElementType == ENavMeshElementType::Edge)
		{
			for (uint32 VertexIndex = 0; VertexIndex < VertexCountPerEdge; VertexIndex++)
			{
				Colors[SelectedElement * VertexCountPerEdge + VertexIndex].R = 255;
			}
		}

		Visualizer->CreateMeshSection_Color(VISUALIZER_SLOT_EDGE, VisualizerMaterial, Positions, Indices, Colors);
	}

	bool NavMeshComponent::FindPathPortals( uint32 StartPlane, uint32 EndPlane, std::vector<uint32>& PathProtals )
	{
		struct PathNode
		{
			PathNode(uint32 InPlaneIndex)
				: PlaneIndex(InPlaneIndex)
				, F(0.0f)
			{}

			uint32 PlaneIndex;
			float F; // path cost

			bool operator>( const PathNode& Other ) const { return F > Other.F; }
			bool operator==( const PathNode& Other ) const { return PlaneIndex == Other.PlaneIndex; }
		};

		const Vector& StartPlaneCenter = ConvexMesh.GetPlane(StartPlane).Center;
		const Vector& EndPlaneCenter = ConvexMesh.GetPlane(EndPlane).Center;

		std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> Frontier;
		std::vector<uint32> PlanesParentProtal(ConvexMesh.NumPlanes(), ConvexMesh.InvalidIndex);
		std::vector<bool> ExploredPlanes(ConvexMesh.NumPlanes(), false);
		std::vector<float> PlanesGValues(ConvexMesh.NumPlanes(), FLT_MAX);

		Frontier.push(StartPlane);
		PlanesGValues[StartPlane] = 0.0f;

		while (!Frontier.empty())
		{
			PathNode CurrentNode = Frontier.top();
			Frontier.pop();

			if (CurrentNode.PlaneIndex == EndPlane)
			{
				while (CurrentNode.PlaneIndex != StartPlane)
				{
					PathProtals.push_back(PlanesParentProtal[CurrentNode.PlaneIndex]);
					CurrentNode.PlaneIndex = ConvexMesh.GetHalfEdgePlane(PlanesParentProtal[CurrentNode.PlaneIndex]);
				}

				//PathPlanes.push_back(StartPlane);
				std::reverse(PathProtals.begin(), PathProtals.end());

				return true;
			}

			const Vector& PlaneCenter = ConvexMesh.GetPlane(CurrentNode.PlaneIndex).Center;
			for (int32 EdgeIndex = 0; EdgeIndex < 3; EdgeIndex++)
			{
				uint32 EdgePlaneIndex = ConvexMesh.GetPlaneHalfEdge(CurrentNode.PlaneIndex, EdgeIndex);
				uint32 TwinIndex = ConvexMesh.GetTwinHalfEdge(EdgePlaneIndex);
				uint32 TwinPlaneIndex = TwinIndex == ConvexMesh.InvalidIndex ? ConvexMesh.InvalidIndex : ConvexMesh.GetHalfEdgePlane(TwinIndex);

				if ((TwinPlaneIndex == ConvexMesh.InvalidIndex) || ExploredPlanes[TwinPlaneIndex])
				{
					continue;
				}

				const Vector& TwinPlaneCenter = ConvexMesh.GetPlane(TwinPlaneIndex).Center;
				float DistanceFromPreviousPlane = Vector::DistSquared(PlaneCenter, TwinPlaneCenter);
				float NewG = PlanesGValues[CurrentNode.PlaneIndex] + DistanceFromPreviousPlane;

				if (NewG < PlanesGValues[TwinPlaneIndex])
				{
					PlanesGValues[TwinPlaneIndex] = NewG;

					PathNode NewNode(TwinPlaneIndex);
					float H = Vector::DistSquared(TwinPlaneCenter, EndPlaneCenter);
					NewNode.F = NewG + H;

					PlanesParentProtal[TwinPlaneIndex] = EdgePlaneIndex;
					Frontier.push(NewNode);
				}
			}
		}

		return false;
	}

	void NavMeshComponent::FunnelPath( const Vector& Start, const Vector& End, std::vector<Vector>& PathPoints, const std::vector<uint32>& PathPortals )
	{
		auto GetPortalLeftVertex = [&](uint32 PortalIndex) { return ConvexMesh.GetHalfEdgeVertex(PathPortals[PortalIndex]); };
		auto GetPortalRightVertex = [&](uint32 PortalIndex) { return ConvexMesh.GetHalfEdgeVertex(ConvexMesh.GetNextHalfEdge(PathPortals[PortalIndex])); };

		PathPoints.reserve(PathPortals.size());
		PathPoints.push_back(Start);

		if (PathPortals.size() > 0)
		{
			auto GetAngle = [&](const Vector& A, const Vector& B, const Vector& C)
			{
				const Vector2 AB = Vector2(B.X - A.X, B.Z - A.Z);
				const Vector2 AC = Vector2(C.X - A.X, C.Z - A.Z);
				return Vector2::CrossProduct(AB, AC);
			};

			const uint64 NumPortals = PathPortals.size() + 2;
			std::vector<Vector> Portals(NumPortals * 2);

			Portals[0] = Start;
			Portals[1] = Start;

			for (uint32 PortalIndex = 0; PortalIndex < PathPortals.size(); PortalIndex++)
			{
				Portals[PortalIndex * 2 + 2] = ConvexMesh.GetVertex(GetPortalLeftVertex(PortalIndex)).Position;
				Portals[PortalIndex * 2 + 3] = ConvexMesh.GetVertex(GetPortalRightVertex(PortalIndex)).Position;
			}

			Portals[NumPortals * 2 - 2] = End;
			Portals[NumPortals * 2 - 1] = End;

			Vector PortalApex = Start;
			Vector PortalLeft = Start;
			Vector PortalRight = Start;

			uint32 LeftIndex = ConvexMesh.InvalidIndex;
			uint32 RightIndex = ConvexMesh.InvalidIndex;

			for (uint32 PortalIndex = 0; PortalIndex < NumPortals; PortalIndex++)
			{
				Vector Left = Portals[PortalIndex * 2];
				Vector Right = Portals[PortalIndex * 2 + 1];
				
				if (PortalApex.NearlyEquals(PortalRight) || PortalApex.NearlyEquals(PortalLeft))
				{
					PortalLeft = Left;
					LeftIndex = PortalIndex;
				}

				else if (GetAngle( PortalApex, Left, PortalLeft ) > 0)
				{
					if (GetAngle(PortalApex, PortalRight, Left) < 0.0f)
					{
						PathPoints.push_back(PortalRight);
					
						PortalApex = PortalRight;
						PortalLeft = PortalApex;
						PortalRight = PortalApex;

						PortalIndex = RightIndex;

						LeftIndex = PortalIndex;
						RightIndex = PortalIndex;

						continue;
					}
					else
					{
						PortalLeft = Left;
						LeftIndex = PortalIndex;
					}
				}

				if (PortalApex.NearlyEquals(PortalLeft) || PortalApex.NearlyEquals(PortalRight))
				{
					PortalRight = Right;
					RightIndex = PortalIndex;
				}

				else if (GetAngle(PortalApex, Right, PortalRight) < 0.0f)
				{
					if (GetAngle(PortalApex, Right, PortalLeft) < 0.0f)
					{
						PathPoints.push_back(PortalLeft);
					
						PortalApex = PortalLeft;
						PortalLeft = PortalApex;
						PortalRight = PortalApex;

						PortalIndex = LeftIndex;

						LeftIndex = PortalIndex;
						RightIndex = PortalIndex;

						continue;
					}
					else
					{
						PortalRight = Right;
						RightIndex = PortalIndex;
					}
				}

				//GetWorld()->DrawDebugSphere(GetWorldTransform().TransformPosition(PortalLeft), Quat::Identity, Color::Cyan, 0.5f, 32, 0, 200);
				//GetWorld()->DrawDebugSphere(GetWorldTransform().TransformPosition(PortalRight), Quat::Identity, Color::Red, 0.5f, 32, 0, 200);
			}
		}

		PathPoints.push_back(End);
	}

	bool NavMeshComponent::FindPath( const Vector& Start, const Vector& End, float AgentRadius, float AgentHeight, std::vector<Vector>& PathPoints, std::vector<uint32>& PathPortals )
	{
		PathPoints.clear();
		PathPortals.clear();

		const Transform CompTransform = GetWorldTransform();

		const Vector StartRelative = CompTransform.InverseTransformPosition(Start);
		const Vector EndRelative = CompTransform.InverseTransformPosition(End);

		bool bStartOverlaps = false;
		Vector StartNearest = Vector::ZeroVector;
		uint32 StartPlane = ConvexMesh.FindNearestPlane(StartRelative, StartNearest, AgentRadius, AgentHeight, bStartOverlaps);

		bool bEndOverlaps = false;
		Vector EndNearest = Vector::ZeroVector;
		uint32 EndPlane = ConvexMesh.FindNearestPlane(EndRelative, EndNearest, AgentRadius, AgentHeight, bEndOverlaps);

		if ((StartPlane == ConvexMesh.InvalidIndex) || (EndPlane == ConvexMesh.InvalidIndex))
		{
			return false;
		}

		bool bFoundPath = FindPathPortals(StartPlane, EndPlane, PathPortals);
		if (!bFoundPath)
		{
			return false;
		}

		FunnelPath(StartNearest, EndNearest, PathPoints, PathPortals);

		for (Vector& Point : PathPoints)
		{
			Point = CompTransform.TransformPosition(Point);
		}
	}

#if WITH_EDITOR
	void NavMeshComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		DrawInternal();
	}

	void NavMeshComponent::DrawInternal()
	{
		ImGui::InputText("Name", &Name);

		if (ImGui::Button("Press"))
		{
			std::vector<Vector> Pos = 
			{
				Vector(1.0f, 0.0f, 0.0f),
				Vector(0.0f, 0.0f, 0.0f),
				Vector(0.0f, 0.0f, 1.0f),
			};

			std::vector<Vector> Pos2 = 
			{
				Vector(7.0f, 0.0f, 0.0f),
				Vector(5.0f, 0.0f, 0.0f),
				Vector(0.0f, 0.0f, 3.0f),
			};


			ConvexMesh.AddTriangle(Pos);
			ConvexMesh.AddTriangle(Pos2);

			UpdateVisualizer();
		}

		if ( ImGui::Button( "Clear" ) )
		{
			ConvexMesh.Clear();
			SelectedElementType = ENavMeshElementType::Invalid;
			UpdateVisualizer();
		}

		if (ImGui::Button("Rebuild"))
		{
			ConvexMesh.Rebuild();
		}

		if (SelectedElementType != ENavMeshElementType::Invalid && ImGui::IsKeyPressed(ImGuiKey_X))
		{
			if (SelectedElementType == ENavMeshElementType::Face)
			{
				ConvexMesh.DeletePlane(SelectedElement);
			}

			else if (SelectedElementType == ENavMeshElementType::Vertex)
			{
				ConvexMesh.DeleteVertex(SelectedElement);
			}

			else if (SelectedElementType == ENavMeshElementType::Edge)
			{
				ConvexMesh.DeleteEdge(SelectedElement);
			}

			SelectedElementType = ENavMeshElementType::Invalid;
			UpdateVisualizer();
		}

		if (SelectedElementType == ENavMeshElementType::Edge && ImGui::IsKeyPressed(ImGuiKey_B))
		{
			ConvexMesh.FillEdge(SelectedElement);
			
			UpdateVisualizer();
		}

		if (SelectedElementType == ENavMeshElementType::Face)
		{
			const Transform CompTransform = GetWorldTransform();

			auto DrawArrow = [&](Vector& Pt0, Vector& Pt1, const Color& Color)
			{
				Pt0 = CompTransform.TransformPosition(Pt0);
				Pt1 = CompTransform.TransformPosition(Pt1);

				const Vector Center = (Pt0 + Pt1) / 2.0f;

				const Vector& Dir = (Pt1 - Pt0).GetSafeNormal();
				const Vector Right = (Dir ^ Vector::UpVector).GetSafeNormal();

				const float XOffset = -0.4f;
				const float YOffset = 1.0f;
				const float LineHalfSize = 1.0f;
				const float ArrowSize = 0.3f;

				const Vector Start = Center - Dir * LineHalfSize + Right * XOffset + Vector::UpVector * YOffset;
				const Vector End = Start + Dir * LineHalfSize * 2.0f;

				GetWorld()->DrawDebugArrow(Start, End, ArrowSize, Color, 0.0f, 0.0f);
			};

			ConvexMesh.VisitPlaneEdges(SelectedElement, [&](uint32 HalfEdgeIndex, uint32 NextHalfEdgeIndex)
			{
				Vector Pt0 = ConvexMesh.GetVertex(ConvexMesh.GetHalfEdgeVertex(HalfEdgeIndex)).Position;
				Vector Pt1 = ConvexMesh.GetVertex(ConvexMesh.GetHalfEdgeVertex(NextHalfEdgeIndex)).Position;

				DrawArrow(Pt0, Pt1, Color::Blue);

				const uint32 TwinIndex = ConvexMesh.GetTwinHalfEdge(HalfEdgeIndex);
				if (TwinIndex != ConvexMesh.InvalidIndex)
				{
					Vector Pt2 = ConvexMesh.GetVertex(ConvexMesh.GetHalfEdgeVertex(TwinIndex)).Position;
					Vector Pt3 = ConvexMesh.GetVertex(ConvexMesh.GetHalfEdgeVertex(ConvexMesh.GetNextHalfEdge(TwinIndex))).Position;

					DrawArrow(Pt2, Pt3, Color::Red);
				}

				return true;
			});
		}
	}

	void NavMeshComponent::SetSelectedInEditor( bool SelectedInEditor, const HitProxyData& Data )
	{
		SceneComponent::SetSelectedInEditor(SelectedInEditor, Data);


	}

	Transform NavMeshComponent::GetGizmoTransformVisualizer() const
	{
		if (SelectedElementType == ENavMeshElementType::Face)
		{
			return ConvexMesh.GetPlaneTransform(SelectedElement) * GetWorldTransform();
		}

		else if (SelectedElementType == ENavMeshElementType::Vertex)
		{
			return ConvexMesh.GetVertexTransform(SelectedElement) * GetWorldTransform();
		}

		else if (SelectedElementType == ENavMeshElementType::Edge)
		{
			return ConvexMesh.GetEdgeTransform(SelectedElement) * GetWorldTransform();
		}

		return GetWorldTransform();
	}

	void NavMeshComponent::OnGizmoTransformChangedVisualizer( const Transform& GizmoTransform, EGizmoSpace Space )
	{
		bool bChanged = false;

		if (SelectedElementType == ENavMeshElementType::Face)
		{
			bChanged = ConvexMesh.SetPlaneTransform(SelectedElement, GizmoTransform.GetRelativeTransform(GetWorldTransform()));
		}

		else if (SelectedElementType == ENavMeshElementType::Vertex)
		{
			bChanged = ConvexMesh.SetVertexTransform(SelectedElement, GizmoTransform.GetRelativeTransform(GetWorldTransform()));
		}

		else if (SelectedElementType == ENavMeshElementType::Edge)
		{
			if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
			{
				uint32 NewVertex = 0;
				const bool bSuccess = ConvexMesh.AddTriangleToEdge(SelectedElement, GizmoTransform.GetRelativeTransform(GetWorldTransform()).GetLocation(), NewVertex);

				if (bSuccess)
				{
					SelectedElementType = ENavMeshElementType::Vertex;
					SelectedElement = NewVertex;
				}
			}

			else
			{
				bChanged = ConvexMesh.SetEdgeTransform(SelectedElement, GizmoTransform.GetRelativeTransform(GetWorldTransform()));
			}
		}

		if (bChanged)
		{
			UpdateVisualizer();
		}
	}

	void NavMeshComponent::SetSelectedInEditorVisualizer( bool SelectedInEditor, const HitProxyData& Data )
	{
		ENavMeshElementType Type = static_cast<ENavMeshElementType>(Data.CustomA);
		uint32 Index = Data.CustomB;

		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && SelectedElementType == ENavMeshElementType::Vertex && Type == ENavMeshElementType::Vertex)
		{
			ConvexMesh.MergePoints(SelectedElement, Index);
			SelectedElement = 0;
			SelectedElementType = ENavMeshElementType::Invalid;
		}
		else
		{
			SelectedElementType = Type;
			SelectedElement = Index;

			if (SelectedElementType == ENavMeshElementType::Invalid)
			{
				SelectedElement = 0;
			}
		}

		UpdateVisualizer();
	}

#endif

// ---------------------------------------------------------------------------------------------------


        }