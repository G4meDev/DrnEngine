#include "DrnPCH.h"
#include "ParticleEmitterType.h"

#if WITH_EDITOR
#include "imgui.h"
#include "Editor/EditorConfig.h"
#endif

namespace Drn
{
	ParticleEmitterType* ParticleEmitterType::Create( EEmitterType Type )
	{
		if (Type == EEmitterType::Sprite_Cpu)
		{
			return new ParticleEmitterCpuSpriteType();
		}

		else if (Type == EEmitterType::Sprite_Gpu)
		{
			return new ParticleEmitterGpuSpriteType();
		}

		else if (Type == EEmitterType::Mesh)
		{
			return new ParticleEmitterMeshType();
		}

		drn_check(false);
		return nullptr;
	}

	ParticleEmitterType* ParticleEmitterType::Create( Archive& Ar )
	{
		drn_check(Ar.IsLoading());

		EEmitterType Type;
		Ar >> *(uint8*)&Type;

		ParticleEmitterType* Out = Create(Type);

		if (Out)
		{
			Out->Serialize(Ar);
		}

		return Out;
	}

// --------------------------------------------------------------------------------------------------------

	ParticleEmitterCpuSpriteType::ParticleEmitterCpuSpriteType()
		: ParticleEmitterType()
	{}

	void ParticleEmitterCpuSpriteType::Serialize( Archive& Ar )
	{
		ParticleEmitterType::Serialize(Ar);

		if (Ar.IsLoading())
		{
			SpriteMaterial.Serialize(Ar);
		}
		else
		{
			SpriteMaterial.Serialize(Ar);
		}
	}

// --------------------------------------------------------------------------------------------------------

	ParticleEmitterMeshType::ParticleEmitterMeshType()
		: ParticleEmitterType()
	{}

	void ParticleEmitterMeshType::Serialize( Archive& Ar )
	{
		ParticleEmitterType::Serialize(Ar);

		if (Ar.IsLoading())
		{
			std::string Path;
			Ar >> Path;
			
			Mesh = AssetHandle<StaticMesh>(Path);
			Mesh.Load();

			uint8 size;
			Ar >> size;
			Materials.clear();
			Materials.resize(size);
			
			for (int32 i = 0; i < size; i++)
			{
				Materials[i].Serialize(Ar);
			}
		}
		else
		{
			Ar << Mesh.GetPath();

			Ar << uint8(Materials.size());
			for (MaterialSlot& OD : Materials)
			{
				OD.Serialize(Ar);
			}
		}
	}

#if WITH_EDITOR
	bool ParticleEmitterType::Draw(TRefCountPtr<ParticleEmitterType>& Ptr)
	{
		const char* const Options[] = { "Sprite_Cpu", "Sprite_Gpu", "Mesh" };
		int32 Selected = (uint8)GetType();
		bool bDirty = ImGui::Combo("Emitter Type", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			Ptr = Create((EEmitterType)Selected);
		}

		return bDirty;
	}

	bool ParticleEmitterMeshType::Draw(TRefCountPtr<ParticleEmitterType>& Ptr)
	{
		bool bDirty = ParticleEmitterType::Draw(Ptr);
		if (bDirty)
		{
			return true;
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

				AssetHandle<Asset> NewMesh(AssetPath);
				EAssetType Type = NewMesh.LoadGeneric();

				if (NewMesh.IsValid() && Type == EAssetType::StaticMesh)
				{
					Mesh = AssetHandle<StaticMesh>(AssetPath);
					Mesh.Load();
					bDirty = true;
				}
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::Separator();
		ImGui::TextWrapped(Mesh.GetPath().c_str());
		ImGui::Separator();

		if (ImGui::Button("Clear"))
		{
			Materials.clear();
			bDirty = true;
		}

		if (ImGui::Button("Add"))
		{
			Materials.push_back({});
			bDirty = true;
		}

		if (ImGui::Button("Remove"))
		{
			Materials.pop_back();
			bDirty = true;
		}

		for (int i = 0; i < Materials.size(); i++)
		{
			bDirty |= Materials[i].Draw(i);
		}

		return bDirty;
	}

	bool ParticleEmitterCpuSpriteType::Draw( TRefCountPtr<ParticleEmitterType>& Ptr )
	{
		bool bDirty = ParticleEmitterType::Draw(Ptr);
		if (bDirty)
		{
			return true;
		}

		bDirty |= SpriteMaterial.Draw(0);

		return bDirty;
	}
#endif

	


        }  // namespace Drn