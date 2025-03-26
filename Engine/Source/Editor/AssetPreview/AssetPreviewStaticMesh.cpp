#include "DrnPCH.h"
#include "AssetPreviewStaticMesh.h"

#if WITH_EDITOR

#include "Editor/Editor.h"
#include "AssetPreviewStaticMeshGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterStaticMesh.h"

namespace Drn
{
	// constructor for opening existing asset menu
	AssetPreviewStaticMesh::AssetPreviewStaticMesh(const std::string InPath)
		: AssetPreview(InPath)
	{
		std::unique_ptr<Archive> Ar = std::make_unique<Archive>( InPath, true );
		Serialize( *Ar.get() );

		GuiLayer = std::make_unique<AssetPreviewStaticMeshGuiLayer>(this);
		GuiLayer->Attach();
	}
	 
	// constructor for creating asset from source
	AssetPreviewStaticMesh::AssetPreviewStaticMesh(const std::string& InPath, const std::string& InSourcePath)
		: AssetPreview(InPath, InSourcePath)
	{
		Reimport();

		Save();
	}

	AssetPreviewStaticMesh::~AssetPreviewStaticMesh()
	{
		if (GuiLayer)
		{
			GuiLayer->DeAttach();
			GuiLayer.reset();
		}
	}

	void AssetPreviewStaticMesh::SetCurrentFocus()
	{
		
	}

	void AssetPreviewStaticMesh::Reimport()
	{
		AssetImporterStaticMesh::Import(this, m_SourcePath);

		RebuildVertexBufferData();
	}

	void AssetPreviewStaticMesh::RebuildVertexBufferData() 
	{
		for (auto& Mesh : MeshesData)
		{
			Mesh.VertexBuffer.clear();

			for (const StaticMeshVertexData& Data : Mesh.Vertices)
			{
				StaticMeshVertexBuffer Buffer;
				Buffer.Pos_X = Data.Pos_X;
				Buffer.Pos_X = Data.Pos_Y;
				Buffer.Pos_X = Data.Pos_Z;

				Buffer.Color_R = Data.Color_R;
				Buffer.Color_G = Data.Color_G;
				Buffer.Color_B = Data.Color_B;

				Mesh.VertexBuffer.push_back(Buffer);
			}
		}
	}

	EAssetType AssetPreviewStaticMesh::GetAssetType()
	{
		return EAssetType::StaticMesh;
	}

	void AssetPreviewStaticMesh::Serialize( Archive& Ar )
	{
		AssetPreview::Serialize(Ar);

		if (Ar.IsLoading())
		{
			MeshesData.clear();

			uint8 size;
			Ar >> size;

			for (int i = 0; i < size; i++)
			{
				StaticMeshData M;
				M.Serialize(Ar);

				MeshesData.push_back(M);
			}


			Ar >> ImportScale;
		}
		
		else
		{
			uint8 size = (uint8)MeshesData.size();
			Ar << size;

			for (int i = 0; i < size; i++)
			{
				MeshesData[i].Serialize(Ar);
			}

			Ar << ImportScale;
		}
	}

	void AssetPreviewStaticMesh::Save()
	{
		std::unique_ptr<Archive> Ar = std::make_unique<Archive>(m_Path, false);
		Serialize(*Ar.get());
	}

	uint8 AssetPreviewStaticMesh::AddMaterial( MaterialData Material )
	{
		for (int i = 0; i < MaterialsData.size(); i++)
		{
			if (MaterialsData[i].Name == Material.Name)
			{
				return i;
			}
		}

		MaterialsData.push_back(Material);
		return MaterialsData.size() - 1;
	}

}

#endif