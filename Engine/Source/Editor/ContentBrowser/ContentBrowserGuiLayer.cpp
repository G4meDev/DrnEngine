#include "DrnPCH.h"
#include "ContentBrowserGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#include "Runtime/Misc/FileSystem.h"
#include "Editor/FileImportMenu/FileImportMenu.h"

LOG_DEFINE_CATEGORY( LogContentBrowser, "ContentBrowser" );

namespace Drn
{
	ContentBrowserGuiLayer::ContentBrowserGuiLayer()
	{
		EngineRootFolder = nullptr;
		GameRootFolder = nullptr;

		SelectedFolder = nullptr;

		OnRefresh();
	}
	
	ContentBrowserGuiLayer::~ContentBrowserGuiLayer()
	{
		
	}

	void ContentBrowserGuiLayer::Draw( float DeltaTime )
	{
		SCOPE_STAT();

		if (!ImGui::Begin("ContentBrowser"))
		{
			ImGui::End();
			return;
		}

		DrawMenuButtons();

		if (ImGui::BeginChild("FolderView", ImVec2(300, 0), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			ImGui::BeginGroup();

			if (EngineRootFolder)
			{
				if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					DrawNextFolder(EngineRootFolder.get());
					ImGui::EndTable();
				}

				if (ImGui::BeginTable("##bg", 1, ImGuiTableFlags_RowBg))
				{
					DrawNextFolder(GameRootFolder.get());
					ImGui::EndTable();
				}
			}

			ImGui::EndGroup();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("FileView", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened))
		{
			if (SelectedFolder)
			{
				DrawFileView();
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}

	void ContentBrowserGuiLayer::DrawNextFolder( SystemFileNode* Node )
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID( Node->File.m_FullPath.c_str() );

		ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
		tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick;
		tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

		if ( Node == SelectedFolder)
			tree_flags |= ImGuiTreeNodeFlags_Selected;

		if (!Node->ContainsDirectory())
		tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

		bool node_open = ImGui::TreeNodeEx( "", tree_flags, "%s", Node->File.m_ShortPath.c_str());

		if (ImGui::IsItemFocused() && SelectedFolder != Node)
		{
			SelectedFolder = Node;
			Selection.Clear();
			Assets.clear();
			
			if (SelectedFolder)
			{
				Assets.reserve(SelectedFolder->GetFiles().size());

				for (SystemFileNode* File : SelectedFolder->GetFiles())
					Assets.emplace_back(File);
			}
		}

		if ( node_open )
		{
			for (SystemFileNode* child : Node->Childs)
			{
				if (child && child->File.m_IsDirectory)
				{
					DrawNextFolder( child );
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void ContentBrowserGuiLayer::DrawFileView()
	{
		//ImGui::SliderFloat("Icon Size")

		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowContentSize(ImVec2(0.0f, LayoutOuterPadding + LayoutLineCount * (LayoutItemSize.y + LayoutItemSpacing)));
		if (ImGui::BeginChild("Assets", ImVec2(0.0f, -ImGui::GetTextLineHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove))
		{
			//ImDrawList* draw_list = ImGui::GetWindowDrawList();

			const float avail_width = ImGui::GetContentRegionAvail().x;
			UpdateLayoutSizes( avail_width );

			ImVec2 start_pos = ImGui::GetCursorScreenPos();
			start_pos        = ImVec2( start_pos.x + LayoutOuterPadding, start_pos.y + LayoutOuterPadding );
			ImGui::SetCursorScreenPos( start_pos );

			ImGuiMultiSelectFlags ms_flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid;

			if (AllowBoxSelect)
				ms_flags |= ImGuiMultiSelectFlags_BoxSelect2d;

			if (AllowDragUnselected)
				ms_flags |= ImGuiMultiSelectFlags_SelectOnClickRelease;

			ms_flags |= ImGuiMultiSelectFlags_NavWrapX;
			ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(ms_flags, Selection.Size, Assets.size());

			// Use custom selection adapter: store ID in selection (recommended)
			Selection.UserData = this;
			Selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self_, int idx) { ContentBrowserGuiLayer* self = (ContentBrowserGuiLayer*)self_->UserData; return ImHashData(self->Assets[idx].FullPath.c_str(), 
				std::strlen(self->Assets[idx].FullPath.c_str())); };
			Selection.ApplyRequests(ms_io);

			const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (Selection.Size > 0)) || RequestDelete;
			//const int item_curr_idx_to_focus = want_delete ? Selection.ApplyDeletionPreLoop( ms_io, Items.Size ) : -1;
			const int item_curr_idx_to_focus = -1;
			RequestDelete = false;

			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing,
				ImVec2( LayoutSelectableSpacing, LayoutSelectableSpacing ) );

			const int column_count = LayoutColumnCount;
			ImGuiListClipper clipper;
			clipper.Begin(LayoutLineCount, LayoutItemStep.y);
			if (item_curr_idx_to_focus != -1)
				clipper.IncludeItemByIndex(item_curr_idx_to_focus / column_count); // Ensure focused item line is not clipped.
			if (ms_io->RangeSrcItem != -1)
				clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem / column_count); // Ensure RangeSrc item line is not clipped.

			while (clipper.Step())
			{
				for (int line_idx = clipper.DisplayStart; line_idx < clipper.DisplayEnd; line_idx++)
				{
					const int item_min_idx_for_current_line = line_idx * column_count;
					const int item_max_idx_for_current_line = std::min((line_idx + 1) * column_count, (int32)Assets.size());
					for (int item_idx = item_min_idx_for_current_line; item_idx < item_max_idx_for_current_line; ++item_idx)
					{
						AssetData& item_data = Assets[item_idx];
						//ImGui::PushID((int)item_data->ID);
						ImGui::PushID(item_data.FullPath.c_str());

						// Position item
						ImVec2 pos = ImVec2(start_pos.x + (item_idx % column_count) * LayoutItemStep.x, start_pos.y + line_idx * LayoutItemStep.y);
						ImGui::SetCursorScreenPos(pos);

						ImGui::SetNextItemSelectionUserData(item_idx);
						//bool item_is_selected = Selection.Contains(ImGuiID(item_data->File.m_FullPath.c_str()));
						bool item_is_selected = Selection.Contains(ImHashData(item_data.FullPath.c_str(), std::strlen(item_data.FullPath.c_str())));
						bool item_is_visible = ImGui::IsRectVisible(LayoutItemSize);
						if (ImGui::Selectable("", item_is_selected, ImGuiSelectableFlags_AllowDoubleClick, LayoutItemSize) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							Editor::Get()->OnSelectedFile( item_data.FullPath );

						// Update our selection state immediately (without waiting for EndMultiSelect() requests)
						// because we use this to alter the color of our text/icon.
						if (ImGui::IsItemToggledSelection())
							item_is_selected = !item_is_selected;

						// Focus (for after deletion)
						if (item_curr_idx_to_focus == item_idx)
							ImGui::SetKeyboardFocusHere(-1);

						if (ImGui::BeginDragDropSource())
						{
							if ( ImGui::GetDragDropPayload() == NULL )
							{
								ImGui::SetDragDropPayload( EditorConfig::Payload_AssetPath(), item_data.FullPath.c_str(), item_data.FullPath.size() + 1);
							}

							const ImGuiPayload* Payload = ImGui::GetDragDropPayload();
							auto PayloadAssets = static_cast<const char *>(Payload->Data);

							ImGui::Text( "1 asset selected:\n \t%s", PayloadAssets);

							ImGui::EndDragDropSource();
						}

						// Render icon (a real app would likely display an image/thumbnail here)
						// Because we use ImGuiMultiSelectFlags_BoxSelect2d, clipping vertical may occasionally be larger, so we coarse-clip our rendering as well.
						if (item_is_visible)
						{
							DrawItem(pos, item_data);
						}

						ImGui::PopID();
					}
				}
			}
			clipper.End();
			ImGui::PopStyleVar(); // ImGuiStyleVar_ItemSpacing

			// Context menu
			if (ImGui::BeginPopupContextWindow())
			{
				ImGui::Text("Selection: %d items", Selection.Size);
				ImGui::Separator();
				if (ImGui::MenuItem("Delete", "Del", false, Selection.Size > 0))
					RequestDelete = true;
				ImGui::EndPopup();
			}

			ms_io = ImGui::EndMultiSelect();
			Selection.ApplyRequests(ms_io);
			//if (want_delete)
			//    Selection.ApplyDeletionPostLoop(ms_io, Items, item_curr_idx_to_focus);

			// Zooming with CTRL+Wheel
			if (ImGui::IsWindowAppearing())
				ZoomWheelAccum = 0.0f;
			if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsAnyItemActive() == false)
			{
				ZoomWheelAccum += io.MouseWheel;
				if (fabsf(ZoomWheelAccum) >= 1.0f)
				{
					// Calculate hovered item index from mouse location
					// FIXME: Locking aiming on 'hovered_item_idx' (with a cool-down timer) would ensure zoom keeps on it.
					const float hovered_item_nx = (io.MousePos.x - start_pos.x + LayoutItemSpacing * 0.5f) / LayoutItemStep.x;
					const float hovered_item_ny = (io.MousePos.y - start_pos.y + LayoutItemSpacing * 0.5f) / LayoutItemStep.y;
					const int hovered_item_idx = ((int)hovered_item_ny * LayoutColumnCount) + (int)hovered_item_nx;
					//ImGui::SetTooltip("%f,%f -> item %d", hovered_item_nx, hovered_item_ny, hovered_item_idx); // Move those 4 lines in block above for easy debugging

					// Zoom
					IconSize *= powf(1.1f, (float)(int)ZoomWheelAccum);
					IconSize = std::clamp(IconSize, IconSizeMin, IconSizeMax);
					ZoomWheelAccum -= (int)ZoomWheelAccum;
					UpdateLayoutSizes(avail_width);

					// Manipulate scroll to that we will land at the same Y location of currently hovered item.
					// - Calculate next frame position of item under mouse
					// - Set new scroll position to be used in next ImGui::BeginChild() call.
					float hovered_item_rel_pos_y = ((float)(hovered_item_idx / LayoutColumnCount) + fmodf(hovered_item_ny, 1.0f)) * LayoutItemStep.y;
					hovered_item_rel_pos_y += ImGui::GetStyle().WindowPadding.y;
					float mouse_local_y = io.MousePos.y - ImGui::GetWindowPos().y;
					ImGui::SetScrollY(hovered_item_rel_pos_y - mouse_local_y);
				}
			}
		}

		ImGui::EndChild();

		ImGui::Text("Selected: %d/%d items", Selection.Size, Assets.size());
	}

	void ContentBrowserGuiLayer::DrawItem(const ImVec2& pos, const AssetData& item_data)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		ImVec2 box_min(pos.x - 1, pos.y - 1);
		ImVec2 box_max(box_min.x + LayoutItemSize.x + 2, box_min.y + LayoutItemSize.y + 2); // Dubious
		//draw_list->AddRectFilled(box_min, box_max, icon_bg_color); // Background color

		// TODO: maybe cache in asset data
		const AssetHandle<Texture2D>& ItemIcon = EditorConfig::GetAssetTypeIcon(item_data.AssetType);
		draw_list->AddImage(ItemIcon->GetRenderTexture()->GetShaderResourceView()->GetDescriptor().GetGpuHandle().ptr, box_min, box_max);

		if (ShowTypeOverlay)
		{
			draw_list->AddRectFilled(ImVec2(box_min.x, box_max.y), ImVec2(box_max.x, box_max.y - icon_type_overlay_size.y), EditorConfig::GetAssetTypeColor(item_data.AssetType).DWColor());
		}

		const bool   display_label              = ( LayoutItemSize.x >= ImGui::CalcTextSize( "999" ).x );
		bool item_is_selected = Selection.Contains(ImHashData(item_data.FullPath.c_str(), std::strlen(item_data.FullPath.c_str())));
		if (display_label)
		{
			ImU32 label_col = ImGui::GetColorU32(item_is_selected ? ImGuiCol_Text : ImGuiCol_TextDisabled);

			ImVec4 ClipRect(box_min.x, box_max.y - ImGui::GetFontSize() - 2 * icon_type_overlay_size.y, box_max.x, box_max.y);
			draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(box_min.x, box_max.y - ImGui::GetFontSize() - 2 * icon_type_overlay_size.y), label_col, item_data.Label.c_str(), NULL, 0.0f, &ClipRect);
		}
	}

	void ContentBrowserGuiLayer::UpdateLayoutSizes( float avail_width )
	{
		LayoutItemSpacing = (float)IconSpacing;
		if (StretchSpacing == false)
			avail_width += floorf(LayoutItemSpacing * 0.5f);
		
		LayoutItemSize = ImVec2(floorf(IconSize), floorf(IconSize));
		LayoutColumnCount = std::max((int)(avail_width / (LayoutItemSize.x + LayoutItemSpacing)), 1);
		LayoutLineCount = (Assets.size() + LayoutColumnCount - 1) / LayoutColumnCount;
		
		if (StretchSpacing && LayoutColumnCount > 1)
			LayoutItemSpacing = floorf(avail_width - LayoutItemSize.x * LayoutColumnCount) / LayoutColumnCount;
		
		LayoutItemStep = ImVec2(LayoutItemSize.x + LayoutItemSpacing, LayoutItemSize.y + LayoutItemSpacing);
		LayoutSelectableSpacing = std::max(floorf(LayoutItemSpacing) - IconHitSpacing, 0.0f);
		LayoutOuterPadding = floorf(LayoutItemSpacing * 0.5f);
	}

	void ContentBrowserGuiLayer::DrawMenuButtons()
	{
		if (ImGui::Button( "Import" ))
			OnImport();
		
		ImGui::SameLine();

		if (ImGui::Button( "Refresh" ))
			OnRefresh();

		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			ImGui::OpenPopup("Add Popup");
		}
		
		if (ImGui::BeginPopup("Add Popup"))
		{
			if (ImGui::Button("Level"))
			{
				ImGui::OpenPopup("Level Popup");
			}

			if (ImGui::BeginPopup("Level Popup"))
			{
				if (ImGui::Button("Empty Level"))
				{
					ImGui::CloseCurrentPopup();
					AddEmptyLevel();
				}

				ImGui::EndPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ContentBrowserGuiLayer::OnImport()
	{
		LOG(LogContentBrowser, Info, "Import");

		Editor::Get()->OpenImportMenu("Select file to import", FileImportMenu::FileFilter_Any(), std::bind(&ContentBrowserGuiLayer::OnSelectedFileToImport, this, std::placeholders::_1));
	}

	void ContentBrowserGuiLayer::OnRefresh()
	{
		LOG(LogContentBrowser, Info, "Refresh");
	
		std::string CachedSelectedDirectoryPath = SelectedFolder ? SelectedFolder->File.m_FullPath : "InvalidPath";

		FileSystem::GetFilesInDirectory( Path::GetProjectPath() + "\\Engine\\Content", EngineRootFolder, ".drn" );
		FileSystem::GetFilesInDirectory( Path::GetProjectPath() + "\\Game\\Content", GameRootFolder, ".drn" );

		EngineRootFolder->File.m_ShortPath = "Engine";
		GameRootFolder->File.m_ShortPath = "Game";

		ConvertPath(EngineRootFolder.get());
		ConvertPath(GameRootFolder.get());

		if (!EngineRootFolder)
		{
			LOG(LogContentBrowser, Error, "failed to load engine content folder.");
		}

		if (!GameRootFolder)
		{
			LOG(LogContentBrowser, Error, "failed to load game content folder.");
		}

		SelectedFolder = nullptr;

		GetDirectoryWithPath( EngineRootFolder.get(), CachedSelectedDirectoryPath, &SelectedFolder);
		if (!SelectedFolder)
		{
			GetDirectoryWithPath( GameRootFolder.get(), CachedSelectedDirectoryPath, &SelectedFolder);
		}
		
		Selection.Clear();
		Assets.clear();
		if (SelectedFolder)
		{
			Assets.reserve(SelectedFolder->GetFiles().size());

			for (SystemFileNode* File : SelectedFolder->GetFiles())
				Assets.emplace_back(File);
		}

		//RequestSort = true;
	}

	void ContentBrowserGuiLayer::OnSelectedFileToImport( std::string FilePath )
	{
		LOG( LogContentBrowser, Info, "selected file to import.\n\t%s ", FilePath.c_str());

		if (!SelectedFolder)
		{
			LOG( LogContentBrowser, Error, "there is no folder selected in content browser for imprting file. ");
			return;
		}
		
		AssetManager::Get()->Create( FilePath, SelectedFolder->File.m_FullPath );
		OnRefresh();
	}

	void ContentBrowserGuiLayer::ConvertPath( SystemFileNode* Node )
	{
		Node->File.m_FullPath = Node->File.m_FullPath.substr(9, Node->File.m_FullPath.size() - 9);

		for (SystemFileNode* Child : Node->Childs)
		{
			ConvertPath(Child);
		}
	}

	void ContentBrowserGuiLayer::GetDirectoryWithPath(SystemFileNode* Node, const std::string& Path ,SystemFileNode** Result)
	{
		if (Node->File.m_FullPath == Path)
		{
			*Result = Node;
			return;
		}

		for (SystemFileNode* Child : Node->Childs)
		{
			if (Child->File.m_IsDirectory)
			{
				GetDirectoryWithPath(Child, Path, Result);
			}
		}
	}

	void ContentBrowserGuiLayer::AddEmptyLevel()
	{
		if (!SelectedFolder)
		{
			LOG(LogContentBrowser, Warning, "Cloudn`t add level. there is no folder selected.");
			return;
		}

		AssetManager::Get()->Create<Level>(SelectedFolder->File.m_FullPath, "Level_", 1);

		OnRefresh();
	}


	AssetData::AssetData( SystemFileNode* FileNode )
	{
		FullPath = FileNode->File.m_FullPath;
		Label = FileNode->File.m_ShortPath.substr(0, FileNode->File.m_ShortPath.size() - 4);

		FileArchive Ar(Path::ConvertProjectPath(FullPath));
		if (Ar.IsValid())
		{
			uint16 TypeByte;
			Ar >> TypeByte;

			AssetType = static_cast<EAssetType>(TypeByte);
		}
	}

}

#endif