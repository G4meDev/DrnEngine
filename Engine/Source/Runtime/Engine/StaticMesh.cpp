#include "DrnPCH.h"
#include "StaticMesh.h"

#include "Editor/AssetPreview/AssetPreviewStaticMeshGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterStaticMesh.h"

LOG_DEFINE_CATEGORY( LogStaticMesh, "StaticMesh" )

using namespace DirectX;

namespace Drn
{
	StaticMesh::StaticMesh(const std::string& InPath)
		: Asset(InPath)
		, m_LoadedOnGPU(false)
	{
		Load();
	}

#if WITH_EDITOR
	StaticMesh::StaticMesh( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_LoadedOnGPU(false)
	{
		m_SourcePath = InSourcePath;

		Import();
		Save();
	}
#endif

	StaticMesh::~StaticMesh()
	{
#if WITH_EDITOR
		CloseAssetPreview();
#endif
	}

	void StaticMesh::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> m_SourcePath;
			Data.Serialize( Ar );
			Ar >> ImportScale;
			
			m_BodySetup.Serialize(Ar);
		}

#if WITH_EDITOR
		else
		{
			Ar << m_SourcePath;
			Data.Serialize( Ar );
			Ar << ImportScale;

			m_BodySetup.Serialize(Ar);
		}
#endif
	}

	void StaticMesh::UploadResources( dx12lib::CommandList* CommandList )
	{
		for (int i = 0; i < Data.MeshesData.size(); i++)
		{
			StaticMeshSlotData& Proxy = Data.MeshesData[i];

			Proxy.VertexBuffer = CommandList->CopyVertexBuffer( Proxy.VertexBufferBlob->GetBufferSize() / sizeof(VertexData_StaticMesh), sizeof(VertexData_StaticMesh), Proxy.VertexBufferBlob->GetBufferPointer());
			Proxy.IndexBuffer = CommandList->CopyIndexBuffer( Proxy.IndexBufferBlob->GetBufferSize() / sizeof(uint32), DXGI_FORMAT_R32_UINT, Proxy.IndexBufferBlob->GetBufferPointer());

#if D3D12_Debug_INFO

			std::string MeshName = m_Path;
			MeshName = Path::ConvertShortPath(MeshName);
			MeshName = Path::RemoveFileExtension(MeshName);
		
			Proxy.VertexBuffer->SetName( StringHelper::s2ws(std::string("VertexBuffer_") + MeshName) );
			Proxy.IndexBuffer->SetName( StringHelper::s2ws(std::string("IndexBuffer_") + MeshName) );

#endif
		}

		m_LoadedOnGPU = true;
	}

	EAssetType StaticMesh::GetAssetType()
	{
		return EAssetType::StaticMesh;
	}

#if WITH_EDITOR

	void StaticMesh::Import()
	{
		AssetImporterStaticMesh::Import(this, m_SourcePath);
		Save();
		Load();

		m_LoadedOnGPU = false;
	}

	void StaticMesh::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewStaticMeshGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void StaticMesh::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

#endif

// ----------------------------------------------------------------------------------------------------------

	void StaticMeshData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			MeshesData.clear();
			Materials.clear();

			uint8 size;

			Ar >> size;
			MeshesData.resize(size);
			for (int i = 0; i < size; i++)
			{
				StaticMeshSlotData& Slot = MeshesData[i];
				Slot.Serialize(Ar);
			}

			Ar >> size;
			Materials.resize(size);
			for (int i = 0; i < size; i++)
			{
				MaterialData& Mat = Materials[i];
				Mat.Serialize(Ar);
			}
		}

		else
		{
			uint8 size = MeshesData.size();
			Ar << size;

			for (int i = 0; i < size; i++)
			{
				MeshesData[i].Serialize(Ar);
			}

			size = Materials.size();
			Ar << size;
			for (int i = 0; i < size; i++)
			{
				Materials[i].Serialize(Ar);
			}
		}
		
	}

	void StaticMeshSlotData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> VertexBufferBlob;
			Ar >> IndexBufferBlob;
			Ar >> MaterialIndex;

			std::vector<VertexData_StaticMesh> D;
			D.resize(VertexBufferBlob->GetBufferSize() / sizeof(VertexData_StaticMesh));
			memcpy(D.data(), VertexBufferBlob->GetBufferPointer(), VertexBufferBlob->GetBufferSize());
			D.pop_back();
		}
		else
		{
			Ar << VertexBufferBlob;
			Ar << IndexBufferBlob;
			Ar << MaterialIndex;
		}
	}

}