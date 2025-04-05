#include "DrnPCH.h"
#include "StaticMeshComponent.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Editor/EditorConfig.h"

#endif

namespace Drn
{
	StaticMeshComponent::StaticMeshComponent()
		: PrimitiveComponent()
	{
		
	}

	StaticMeshComponent::~StaticMeshComponent()
	{
		std::cout << "Remove";
	}

	void StaticMeshComponent::Tick( float DeltaTime )
	{
		PrimitiveComponent::Tick(DeltaTime);
	}

	void StaticMeshComponent::SetMesh( const AssetHandle<StaticMesh>& InHandle )
	{
		Mesh = InHandle;
	}

	void StaticMeshComponent::Serialize( Archive& Ar )
	{
		PrimitiveComponent::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string Path;
			Ar >> Path;
			
			Mesh = AssetHandle<StaticMesh>(Path);
			Mesh.Load();
			
			SetMesh(Mesh);
		}
		
		else
		{
			Ar << Mesh.GetPath();
		}
	}

#if WITH_EDITOR

	void StaticMeshComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		if ( ImGui::Button( "Clear" ) )
		{
			ClearMesh();
		}

		std::string AssetPath	= Mesh.GetPath();
		std::string AssetName	= Path::ConvertShortPath(AssetPath);
		AssetName				= Path::RemoveFileExtension(AssetName);
		AssetName				= AssetName == "" ? "None" : AssetName;

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, EditorConfig::AssetInputColor);
		ImGui::Text( "%s", AssetName.c_str() );
		ImGui::PopStyleColor();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
			{
				auto AssetPath = static_cast<const char*>(payload->Data);
				UpdateMeshWithPath(AssetPath);
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();

		ImGui::TextWrapped(Mesh.GetPath().c_str());
	}

	void StaticMeshComponent::ClearMesh()
	{
		Mesh = AssetHandle<StaticMesh>("");
	}

	void StaticMeshComponent::UpdateMeshWithPath( const char* NewPath )
	{
		Mesh = AssetHandle<StaticMesh>(NewPath);
		Mesh.Load();
	}

#endif
}