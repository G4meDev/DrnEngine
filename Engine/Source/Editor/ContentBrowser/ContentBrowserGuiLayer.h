#pragma once

#if WITH_EDITOR
#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY( LogContentBrowser );

namespace Drn
{
	class ContentBrowserGuiLayer: public ImGuiLayer
	{
	public:
		ContentBrowserGuiLayer();
		~ContentBrowserGuiLayer();

		virtual void Draw( float DeltaTime ) override;

	private:

		void DrawNextFolder(SystemFileNode* Node);
		void DrawFileView();
		void DrawItem(const ImVec2& pos, SystemFileNode* item_data);
		void UpdateLayoutSizes(float avail_width);

		void DrawMenuButtons();

		void OnImport();
		void OnRefresh();

		void OnSelectedFileToImport(std::string FilePath);

		void ConvertPath(SystemFileNode* Node);

		void GetDirectoryWithPath(SystemFileNode* Node, const std::string& Path, SystemFileNode** Result);

		std::unique_ptr<SystemFileNode> EngineRootFolder;
		std::unique_ptr<SystemFileNode> GameRootFolder;

		SystemFileNode* SelectedFolder;
		std::vector<SystemFileNode*> SelectedFolderFiles;

		std::shared_ptr<Window> ImportWindow;

		friend class ContentBrowser;

		void AddEmptyLevel();

		ImGuiSelectionBasicStorage Selection;
		ImGuiID         NextItemId = 0;       
		bool            RequestDelete = false;
		bool            RequestSort = false;  
		float           ZoomWheelAccum = 0.0f;

		bool            ShowTypeOverlay = true;
		bool            AllowSorting = true;
		bool            AllowDragUnselected = false;
		bool            AllowBoxSelect = false;
		float IconSize = 128.0f;
		float IconSizeMin = 64.0f;
		float IconSizeMax = 256.0f;
		int IconSpacing = 10;
		int IconHitSpacing = 4;
		bool StretchSpacing = true;

		ImVec2 LayoutItemSize;
		ImVec2 LayoutItemStep;
		float LayoutItemSpacing = 0.0f;
		float LayoutSelectableSpacing = 0.0f;
		float LayoutOuterPadding = 0.0f;
		int LayoutColumnCount = 0;
		int LayoutLineCount = 0;

		const ImU32 icon_type_overlay_colors[3] = { 0, IM_COL32( 200, 70, 70, 255 ), IM_COL32( 70, 170, 70, 255 ) };
		const ImU32 icon_bg_color               = ImGui::GetColorU32( IM_COL32( 35, 35, 35, 220 ) );
		const ImVec2 icon_type_overlay_size     = ImVec2( 4.0f, 4.0f );
	};
}

#endif