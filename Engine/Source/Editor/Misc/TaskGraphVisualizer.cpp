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

			std::string NodeID;
			std::string NodeName;

			size_t LineStartPos = 0;
			size_t LineEndPos = TaskGraphDump.find("\n");

			while ( LineEndPos != std::string::npos)
			{
				size_t LabelPos = TaskGraphDump.find(NodeLabelTag, LineStartPos);

				if ( LabelPos != std::string::npos && LabelPos < LineEndPos)
				{
					NodeID = TaskGraphDump.substr(LineStartPos, LabelPos - LineStartPos);
					NodeName = TaskGraphDump.substr(LabelPos + 8, LineEndPos - LabelPos - 12);

					//std::cout << NodeName << " " << NodeID << '\n';

					m_GraphDelegate.AddNode(std::move(NodeName), std::move(NodeID));
				}

				LineStartPos = LineEndPos + 1;
				LineEndPos = TaskGraphDump.find("\n", LineStartPos);
			}

			LineStartPos = 0;
			LineEndPos = TaskGraphDump.find("\n");

			std::string StartID;
			std::string EndID;

			while ( LineEndPos != std::string::npos)
			{
				size_t ArrowPos = TaskGraphDump.find(NodeArrowTag, LineStartPos);

				if (ArrowPos != std::string::npos && ArrowPos < LineEndPos)
				{
					StartID = TaskGraphDump.substr(LineStartPos, ArrowPos - LineStartPos);
					EndID = TaskGraphDump.substr(ArrowPos + 4, LineEndPos - ArrowPos - 5);

					//std::cout << StartID << " " << EndID << '\n';

					m_GraphDelegate.AddLink(StartID, EndID);
				}

				LineStartPos = LineEndPos + 1;
				LineEndPos = TaskGraphDump.find("\n", LineStartPos);
			}

			std::set<size_t> SeenTargetNodes;
			for (auto& Node : m_GraphDelegate.mLinks)
			{
				SeenTargetNodes.insert(Node.mOutputNodeIndex);
			}

			std::set<size_t> RootNodes;
			for (size_t i = 0; i < m_GraphDelegate.mNodes.size(); i++)
			{
				if (!SeenTargetNodes.contains(i))
					RootNodes.insert(i);
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

#endif