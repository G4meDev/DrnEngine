#include "DrnPCH.h"
#include "RenderTypes.h"

#if WITH_EDITOR
#include "Editor/EditorConfig.h"
#include <imgui.h>
#endif

namespace Drn
{
	void NamedTexture2DSlot::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_Name;

			std::string Path;
			Ar >> Path;
			m_Texture2D = AssetHandle<Texture2D>(Path);
		}
		else
		{
			Ar << m_Name;
			Ar << m_Texture2D.GetPath();
		}
	}

#if WITH_EDITOR
	bool NamedTexture2DSlot::Draw()
	{
		ImGui::Text(m_Name.c_str());
		ImGui::SameLine();
		ImGui::TextWrapped(m_Texture2D.GetPath().c_str());

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConfig::Payload_AssetPath()))
			{
				auto AssetPath = static_cast<const char*>(payload->Data);

				AssetHandle<Asset> CheckMaterial(AssetPath);
				EAssetType Type = CheckMaterial.LoadGeneric();

				if (CheckMaterial.IsValid() && Type == EAssetType::Texture2D)
				{
					m_Texture2D = AssetHandle<Texture2D>(AssetPath);
					return true;
				}
			}

			ImGui::EndDragDropTarget();
		}

		return false;
	}
#endif
}