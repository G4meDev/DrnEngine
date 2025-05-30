#include "DrnPCH.h"
#include "Runtime/Engine/NamedProperty.h"

#if WITH_EDITOR
#include "Editor/EditorConfig.h"
#include <imgui.h>
#endif

namespace Drn
{
	void NamedProperty::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> m_Name;
		}
		else
		{
			Ar << m_Name;
		}
	}

	void Texture2DProperty::Serialize( Archive& Ar )
	{
		NamedProperty::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string Path;
			Ar >> Path;
			m_Texture2D = AssetHandle<Texture2D>(Path);
		}
		else
		{
			Ar << m_Texture2D.GetPath();
		}
	}

#if WITH_EDITOR
	AssetHandle<Texture2D> Texture2DProperty::Draw()
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
					AssetHandle<Texture2D> Result(AssetPath);
					Result.Load();
					return Result;
				}
			}

			ImGui::EndDragDropTarget();
		}

		return AssetHandle<Texture2D>();
	}
#endif

	void FloatProperty::Serialize( Archive& Ar )
	{
		NamedProperty::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Value;
		}
		else
		{
			Ar << m_Value;
		}
	}

#if WITH_EDITOR
	bool FloatProperty::Draw()
	{
		ImGui::Text(m_Name.c_str());
		ImGui::SameLine();
		ImGui::PushID(m_Name.c_str());
		bool Result = ImGui::SliderFloat("##", &m_Value, -1, 1);
		ImGui::PopID();

		return Result;
	}
#endif

	void MaterialIndexedFloatParameter::Serialize( Archive& Ar )
	{
		FloatProperty::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Index;
		}
		else
		{
			Ar << m_Index;
		}
	}

	void Vector4Property::Serialize( Archive& Ar )
	{
		NamedProperty::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Value;
		}
		else
		{
			Ar << m_Value;
		}
	}

#if WITH_EDITOR
	bool Vector4Property::Draw()
	{
		ImGui::Text(m_Name.c_str());
		ImGui::SameLine();
		ImGui::PushID(m_Name.c_str());
		bool Result = m_Value.Draw();
		ImGui::PopID();

		return Result;
	}
#endif

	void MaterialIndexedVector4Parameter::Serialize( Archive& Ar )
	{
		Vector4Property::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_Index;
		}
		else
		{
			Ar << m_Index;
		}
	}

}