#include "DrnPCH.h"
#include "TaskGraphVisualizer.h"

#if WITH_EDITOR

#include "Runtime/Core/Application.h"

LOG_DEFINE_CATEGORY( LogTaskGraphVisializer, "TaskGraphVisializer" );

const std::regex NodePattern( ".*\\[label=.*\\];" );
const std::regex IDPattern( ".*\\[label=" );
const std::regex NamePattern( "\\[label=.*" );
const std::string NodeLabelTag( "[label=" );
const std::string NodeArrowTag( " -> " );

namespace Drn
{
	TaskGraphVisualizer::TaskGraphVisualizer()
	{
		LOG(LogTaskGraphVisializer, Info, "opening task graph visualizer");
	}
	
	TaskGraphVisualizer::~TaskGraphVisualizer()
	{
		LOG(LogTaskGraphVisializer, Info, "closing task graph visualizer");
	}
	
	void TaskGraphVisualizer::Draw( float DeltaTime )
	{
		SCOPE_STAT(TaskGraphVisualizerDraw);

		if ( ImGui::Begin( "TaskGraphVisualizer", &m_Open ) )
		{
			m_GraphDelegate.Clear();

			const std::string TaskGraphDump = Application::taskflow.dump();
			//Application::taskflow.dump(std::cout);

			std::vector<GvGraph> SubGraphs;
			{
				size_t SubgraphPos = TaskGraphDump.find("subgraph");
				while (SubgraphPos != std::string::npos)
				{
					SubGraphs.push_back({});
					GvGraph& Graph = SubGraphs[SubGraphs.size() - 1];
					SubgraphPos += 10;
					size_t IDEnd = TaskGraphDump.find(" ", SubgraphPos);
					size_t IDLength = IDEnd - SubgraphPos;
					Graph.ID = std::move(TaskGraphDump.substr(SubgraphPos, IDLength));
					SubgraphPos += IDLength;

					size_t SubGraphEnd = TaskGraphDump.find( "}", SubgraphPos );

					SubgraphPos = std::min(TaskGraphDump.find( "\n", SubgraphPos ), SubGraphEnd);
					while (SubgraphPos != SubGraphEnd)
					{
						SubgraphPos++;
						size_t LineEnd = std::min(TaskGraphDump.find( "\n", SubgraphPos ), SubGraphEnd);

						size_t LabelPos = std::min(TaskGraphDump.find( "label=", SubgraphPos ), LineEnd);
						size_t ArrowPos = std::min( TaskGraphDump.find( "->", SubgraphPos ), LineEnd );

						if (LabelPos != LineEnd && SubgraphPos == LabelPos)
						{
							Graph.Label = std::move(TaskGraphDump.substr(SubgraphPos + 7, LineEnd - SubgraphPos - 9));
						}

						else if (LabelPos != LineEnd)
						{
							size_t BracketPos = std::min(TaskGraphDump.find("[", SubgraphPos), LineEnd);
							std::string ID = std::move(TaskGraphDump.substr(SubgraphPos, BracketPos - SubgraphPos));
							SubgraphPos = BracketPos + 1;

							LabelPos = TaskGraphDump.find( "label=", SubgraphPos );

							// e.g. p000002175D253E50[label="WorldTick" ];
							if (BracketPos != LineEnd && LabelPos == SubgraphPos)
							{
								if (!Graph.Nodes.contains(ID))
									Graph.Nodes[ID] = GvNode();
								GvNode& Node = Graph.Nodes[ID];
								Node.Type = GvType::Node;
								Node.ID = std::move(ID);
								Node.Label = std::move(TaskGraphDump.substr(LabelPos + 7, LineEnd - LabelPos - 11 ));
							}
							// e.g. p000002175D252560[shape=box3d, color=blue, label="RendererTick [m1]"];
							else if (BracketPos != LineEnd)
							{
								size_t LabelBracketStart = TaskGraphDump.find("[", SubgraphPos + 1);
								size_t LabelBracketEnd = TaskGraphDump.find("]", SubgraphPos + 1);
								std::string Label = std::move(TaskGraphDump.substr(LabelBracketStart + 1, LabelBracketEnd - LabelBracketStart - 1));

								if (!Graph.Nodes.contains(ID))
									Graph.Nodes[ID] = GvNode();
								GvNode& Node = Graph.Nodes[ID];
								Node.Type = GvType::Graph;
								Node.ID = std::move(ID);
								Node.Label = std::move(Label);
							}
						}

						// e.g. p000002175D253E50 -> p000002175D252560;
						else if (ArrowPos != LineEnd)
						{
							std::string FromID = std::move(TaskGraphDump.substr(SubgraphPos, ArrowPos - SubgraphPos));
							std::string ToID = std::move(TaskGraphDump.substr(ArrowPos + 2, LineEnd - ArrowPos - 3));
							
							StringHelper::RemoveWhitespaces(FromID);
							StringHelper::RemoveWhitespaces(ToID);
							
							if (!Graph.Nodes.contains(FromID))
								Graph.Nodes[FromID] = GvNode();
							GvNode& FromNode = Graph.Nodes[FromID];
							FromNode.Succeed.push_back(ToID);
							
							if (!Graph.Nodes.contains(ToID))
								Graph.Nodes[ToID] = GvNode();
							GvNode& ToNode = Graph.Nodes[ToID];
							ToNode.Precede.push_back(FromID);
						}

						SubgraphPos = LineEnd;
					}

					SubgraphPos = TaskGraphDump.find("subgraph", SubgraphPos);
				}

				GvGraph& MainGraph = SubGraphs[0];
				for (int i = 1; i < SubGraphs.size(); i++)
				{
					SubGraphs[i].RecalculateRootAndLeaf();
				}

				std::string SubgraphID = MainGraph.CheckContainsSubgraph();
				while (SubgraphID != "")
				{
					GvNode& SubgraphNode = MainGraph.Nodes[SubgraphID];
					for (int i = 1; i < SubGraphs.size(); i++)
					{
						if (SubGraphs[i].Label == SubgraphNode.Label)
						{
							GvGraph& Subgraph = SubGraphs[i];

							for ( auto it = Subgraph.Nodes.begin(); it != Subgraph.Nodes.end(); it++ )
							{
								MainGraph.Nodes[it->first] = it->second;
							}

							for (const std::string& PreID : SubgraphNode.Precede)
							{
								GvNode& PreNode = MainGraph.Nodes[PreID];
								PreNode.Succeed.erase(std::remove(PreNode.Succeed.begin(), PreNode.Succeed.end(), SubgraphID), PreNode.Succeed.end());

								for ( const std::string& RootID : Subgraph.Roots )
								{
									MainGraph.Nodes[RootID].Precede.push_back(PreID);
									PreNode.Succeed.push_back(RootID);
								}
							}

							for (const std::string& SucID : SubgraphNode.Succeed)
							{
								GvNode& SucNode = MainGraph.Nodes[SucID];
								SucNode.Precede.erase(std::remove(SucNode.Precede.begin(), SucNode.Precede.end(), SubgraphID), SucNode.Precede.end());

								for ( const std::string& LeafID : Subgraph.Leafs )
								{
									MainGraph.Nodes[LeafID].Succeed.push_back(SucID);
									SucNode.Precede.push_back(LeafID);
								}
							}

							MainGraph.Nodes.erase(SubgraphID);
						}
					}

					SubgraphID = MainGraph.CheckContainsSubgraph();
				}

				for (auto it = MainGraph.Nodes.begin(); it != MainGraph.Nodes.end(); it++)
				{
					m_GraphDelegate.AddNode(std::move(it->second.Label), std::move(it->second.ID));
				}

				for (auto it = MainGraph.Nodes.begin(); it != MainGraph.Nodes.end(); it++)
				{
					for (const std::string Suc : it->second.Succeed)
					{
						m_GraphDelegate.AddLink(it->first, Suc);
					}
				}

				MainGraph.RecalculateRootAndLeaf();
				std::set<size_t> RootNodes;
				for (const std::string& Root : MainGraph.Roots)
				{
					RootNodes.insert( m_GraphDelegate.m_NodeMap[Root] );
				}
				
				uint32 MaxDepth = 0;
				for (auto it = RootNodes.begin(); it != RootNodes.end(); it++)
				{
					uint32 Depth = 0;
					m_GraphDelegate.RecursiveDepth( *it, Depth, MaxDepth);
				}
				
				std::vector<uint32> DepthNodeCount;
				DepthNodeCount.resize(MaxDepth + 1, 0);
				
				for (auto it = m_GraphDelegate.mNodes.begin(); it != m_GraphDelegate.mNodes.end(); it++)
				{
					it->Branch = DepthNodeCount[it->Depth]++;
				
					it->x = it->Depth * 400;
					it->y = it->Branch * 400;
				}
			}

			GraphEditor::Show(m_GraphDelegate, m_GraphOptions, m_GraphViewState, true);
		}
		ImGui::End();
	}

// -----------------------------------------------------------------------------------------------------------------------------

	bool TaskGraphEditorDelegate::AllowedLink( GraphEditor::NodeIndex from, GraphEditor::NodeIndex to )
	{
		return true;
	}

	void TaskGraphEditorDelegate::SelectNode( GraphEditor::NodeIndex nodeIndex, bool selected )
	{
		mNodes[nodeIndex].mSelected = selected;
	}

	void TaskGraphEditorDelegate::MoveSelectedNodes( const ImVec2 delta )
	{
		for ( auto& node: mNodes )
		{
			if ( !node.mSelected )
			{
				continue;
			}
			node.x += delta.x;
			node.y += delta.y;
		}
	}

	void TaskGraphEditorDelegate::RightClick( GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput,
												GraphEditor::SlotIndex slotIndexOutput ) 
	{}

	void TaskGraphEditorDelegate::AddLink( GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex,
											GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex )
	{
		mLinks.push_back( { inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex } );
	}


	void TaskGraphEditorDelegate::DelLink( GraphEditor::LinkIndex linkIndex )
	{
		//mLinks.erase( mLinks.begin() + linkIndex );
	}

	void TaskGraphEditorDelegate::CustomDraw( ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex )
	{
		//drawList->AddLine( rectangle.Min, rectangle.Max, IM_COL32( 0, 0, 0, 255 ) );
		//drawList->AddText( rectangle.Min, IM_COL32( 255, 128, 64, 255 ), "Draw" );
	}

	const size_t TaskGraphEditorDelegate::GetTemplateCount()
	{
		return sizeof( mTemplates ) / sizeof( GraphEditor::Template );
	}

	const GraphEditor::Template TaskGraphEditorDelegate::GetTemplate( GraphEditor::TemplateIndex index )
	{
		return mTemplates[index];
	}

	const size_t TaskGraphEditorDelegate::GetNodeCount()
	{
		return mNodes.size();
	}

	const GraphEditor::Node TaskGraphEditorDelegate::GetNode( GraphEditor::NodeIndex index )
	{
		const auto& myNode = mNodes[index];
		return GraphEditor::Node {
			myNode.name.c_str(), myNode.templateIndex,
			ImRect( ImVec2( myNode.x, myNode.y ), ImVec2( myNode.x + 200, myNode.y + 200 ) ), myNode.mSelected
		};
	}

	const size_t TaskGraphEditorDelegate::GetLinkCount()
	{
		return mLinks.size();
	}

	const GraphEditor::Link TaskGraphEditorDelegate::GetLink( GraphEditor::LinkIndex index )
	{
		return mLinks[index];
	}

	void TaskGraphEditorDelegate::Clear()
	{
		mNodes.clear();
		mLinks.clear();
		m_NodeMap.clear();
	}

	void TaskGraphEditorDelegate::AddNode( std::string&& Name, std::string&& ID )
	{
		m_NodeMap[ID] = mNodes.size();
		mNodes.emplace_back(Node(std::move(Name), std::move(ID), 2, mNodes.size() * 400.0f, mNodes.size() * 400.0f, false));
	}

	void TaskGraphEditorDelegate::AddLink( const std::string& From, const std::string& To )
	{
		mLinks.push_back(GraphEditor::Link(m_NodeMap[From], 0,  m_NodeMap[To], 0));
	}

	void TaskGraphEditorDelegate::RecursiveDepth( const size_t NodeIndex, uint32 Depth, uint32& MaxDepth )
	{
		mNodes[NodeIndex].Depth = std::max(mNodes[NodeIndex].Depth, Depth);
		MaxDepth = std::max(mNodes[NodeIndex].Depth, MaxDepth);

		for ( auto it = mLinks.begin(); it != mLinks.end(); it++ )
		{
			if (it->mInputNodeIndex == NodeIndex)
			{
				RecursiveDepth(it->mOutputNodeIndex, Depth + 1, MaxDepth);
			}
		}
	}

}

void GvGraph::RecalculateRootAndLeaf()
{
	Roots.clear();
	Leafs.clear();

	for (auto it = Nodes.begin(); it != Nodes.end(); it++)
	{
		if (it->second.Precede.size() == 0)
			Roots.push_back(it->first);

		if (it->second.Succeed.size() == 0)
			Leafs.push_back(it->first);
	}
}

std::string GvGraph::CheckContainsSubgraph()
{
	for (auto it = Nodes.begin(); it != Nodes.end(); it++)
	{
		if (it->second.Type == GvType::Graph)
		{
			return it->first;
		}
	}

	return "";
}

void GvGraph::ExpandSubgraph( const std::string& ID ) {}

#endif