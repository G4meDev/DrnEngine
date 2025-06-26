#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"
#include <GraphEditor.h>

LOG_DECLARE_CATEGORY(LogTaskGraphVisializer);

struct GvLink
{
	std::string From;
	std::string To;
};

enum GvType
{
	Node,
	Graph
};

struct GvBase
{
	std::string ID;
	std::string Label;

	std::vector<std::string> Precede;
	std::vector<std::string> Succeed;

};

struct GvNode : public GvBase
{
	GvNode() {}

	GvType Type;
};

struct GvGraph : public GvBase
{
	std::map<std::string, GvNode> Nodes;
	std::vector<std::string> Roots;
	std::vector<std::string> Leafs;

	void RecalculateRootAndLeaf();
	std::string CheckContainsSubgraph();
	void ExpandSubgraph( const std::string& ID );
};


template<typename T, std::size_t N>
struct Array
{
    T            data[N];
    const size_t size() const
    {
        return N;
    }

    const T operator[]( size_t index ) const
    {
        return data[index];
    }
    operator T*()
    {
        T* p = new T[N];
        memcpy( p, data, sizeof( data ) );
        return p;
    }
};

template<typename T, typename... U> Array( T, U... ) -> Array<T, 1 + sizeof...( U )>;

namespace Drn
{
	struct TaskGraphEditorDelegate : public GraphEditor::Delegate
	{
		bool AllowedLink( GraphEditor::NodeIndex from, GraphEditor::NodeIndex to ) override;
		void SelectNode( GraphEditor::NodeIndex nodeIndex, bool selected ) override;
		void MoveSelectedNodes( const ImVec2 delta ) override;
		virtual void RightClick( GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput ) override;
		void AddLink( GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex,
						GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex ) override;
		void DelLink( GraphEditor::LinkIndex linkIndex ) override;
		void CustomDraw( ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex ) override;
		const size_t GetTemplateCount() override;
		const GraphEditor::Template GetTemplate( GraphEditor::TemplateIndex index ) override;
		const size_t GetNodeCount() override;
		const GraphEditor::Node GetNode( GraphEditor::NodeIndex index ) override;
		const size_t GetLinkCount() override;
		const GraphEditor::Link GetLink( GraphEditor::LinkIndex index ) override;

		static const inline GraphEditor::Template mTemplates[] = {
		{
			IM_COL32(160, 160, 180, 255),
			IM_COL32(100, 100, 140, 255),
			IM_COL32(110, 110, 150, 255),
			1,
			Array{"MyInput"},
			nullptr,
			2,
			Array{"MyOutput0", "MyOuput1"},
			nullptr
		},

		{
			IM_COL32(180, 160, 160, 255),
			IM_COL32(140, 100, 100, 255),
			IM_COL32(150, 110, 110, 255),
			3,
			nullptr,
			Array{ IM_COL32(200,100,100,255), IM_COL32(100,200,100,255), IM_COL32(100,100,200,255) },
			1,
			Array{"MyOutput0"},
			Array{ IM_COL32(200,200,200,255)}
		},
		
		{
			IM_COL32(160, 160, 180, 255),
			IM_COL32(100, 100, 140, 255),
			IM_COL32(110, 110, 150, 255),
			1,
			nullptr,
			nullptr,
			1,
			nullptr,
			nullptr
		},
		
		};

		struct Node
		{
			Node(std::string&& InName, std::string&& InID, GraphEditor::TemplateIndex Index, float InX, float InY, bool Selected)
				: templateIndex(Index)
				, x(InX)
				, y(InY)
				, mSelected(Selected)
				, Depth(0)
				, Branch(0)
			{
				name = std::move(InName);
				ID = std::move(InID);
			}

			std::string                name;
			std::string ID;
			GraphEditor::TemplateIndex templateIndex;
			float                      x, y;
			bool                       mSelected;
			uint32 Depth;
			uint32 Branch;
		};

		std::vector<Node> mNodes = {{ "My Node 0", "123", 0, 0, 0, false },
									{ "My Node 1", "123", 0, 400, 0, false },
									{ "My Node 2", "123", 1, 400, 400, false } };

		std::vector<GraphEditor::Link> mLinks = { { 0, 0, 1, 0 } };

		std::unordered_map<std::string, size_t> m_NodeMap;

		void Clear();
		void AddNode( std::string&& Name, std::string&& ID );
		void AddLink( const std::string& From, const std::string& To );

		void RecursiveDepth( const size_t NodeIndex, uint32 Depth, uint32& MaxDepth);
	};

	class TaskGraphVisualizer : public ImGuiLayer
	{
	public:
		TaskGraphVisualizer();
		virtual ~TaskGraphVisualizer();

		virtual void Draw( float DeltaTime ) override;

	private:
		GraphEditor::Options m_GraphOptions;
		TaskGraphEditorDelegate m_GraphDelegate;
		GraphEditor::ViewState m_GraphViewState;

	};
}

#endif