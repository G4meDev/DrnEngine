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
		, GuiLayer(nullptr)
	{
		Load();
	}

	StaticMesh::StaticMesh( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath, InSourcePath)
		, GuiLayer( nullptr )
	{
		Import();
		Save();
	}

	StaticMesh::~StaticMesh()
	{
#if WITH_EDITOR
		CloseAssetPreview();
#endif

		
	}

	void StaticMesh::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			RenderProxies.clear();

			Ar >> m_AssetType;
			Ar >> m_AssetVersion;
			Ar >> m_SourcePath;
			Data.Serialize( Ar );
			Ar >> ImportScale;

			for ( auto& Mesh: Data.MeshesData )
			{
				StaticMeshRenderProxy RP;

				RP.VertexData = Mesh.VertexData;
				RP.IndexData  = Mesh.IndexData;

				RenderProxies.push_back( RP );
			}
		}

#if WITH_EDITOR
		else
		{
			Ar << static_cast<uint8>(GetAssetType());
			Ar << m_AssetVersion;
			Ar << m_SourcePath;
			Data.Serialize( Ar );
			Ar << ImportScale;
		}
#endif
	}

	void StaticMesh::UploadResources( dx12lib::CommandList* CommandList )
	{
		for (auto& Proxy : RenderProxies)
		{
			Proxy.VertexBuffer = CommandList->CopyVertexBuffer( Proxy.VertexData.size(), sizeof(StaticMeshVertexBuffer), Proxy.VertexData.data());
			Proxy.IndexBuffer = CommandList->CopyIndexBuffer( Proxy.IndexData.size(), DXGI_FORMAT_R32_UINT, Proxy.IndexData.data());
		}
	}

	void StaticMesh::Save()
	{
		Archive Ar = Archive(m_Path, false);
		Serialize(Ar);
	}

	void StaticMesh::Load() 
	{
		Archive Ar = Archive(m_Path);
		Serialize(Ar);
	}

	void StaticMesh::Import()
	{
		AssetImporterStaticMesh::Import(this, m_SourcePath);
	}

	EAssetType StaticMesh::GetAssetType()
	{
		return EAssetType::StaticMesh;
	}

#if WITH_EDITOR
	void StaticMesh::OpenAssetPreview()
	{
		GuiLayer = new AssetPreviewStaticMeshGuiLayer( this );
		GuiLayer->Attach();
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

			for (int i = 0; i < size; i++)
			{
				StaticMeshSlotData M;
				M.Serialize(Ar);

				MeshesData.push_back(M);
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
		}
		
	}

	void StaticMeshSlotData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			std::vector<char> Buffer;

			Ar >> Buffer;
			int size =  Buffer.size() / sizeof( StaticMeshVertexBuffer );
			VertexData.resize(size);
			std::memcpy( VertexData.data(), Buffer.data(), Buffer.size() );

			Ar >> Buffer;
			size = Buffer.size() / sizeof(uint32);
			IndexData.resize(size);
			std::memcpy( IndexData.data(), Buffer.data(), Buffer.size() );

			Ar >> Stride;
			Ar >> MaterialIndex;
		}
		else
		{
			uint64 size = VertexData.size() * sizeof( StaticMeshVertexBuffer );
			std::vector<char> buffer( size );
			std::memcpy( buffer.data(), VertexData.data(), size );
			Ar << buffer;

			size = IndexData.size() * sizeof( uint32 );
			buffer.resize( size );
			std::memcpy( buffer.data(), IndexData.data(), size );
			Ar << buffer;

			Ar << Stride;
			Ar << MaterialIndex;
		}
	}
}