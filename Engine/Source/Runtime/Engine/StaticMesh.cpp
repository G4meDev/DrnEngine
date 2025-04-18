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
			RenderProxies.clear();

			Ar >> m_SourcePath;
			Data.Serialize( Ar );
			Ar >> ImportScale;
			
			m_BodySetup.Serialize(Ar);


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
			Ar << m_SourcePath;
			Data.Serialize( Ar );
			Ar << ImportScale;

			m_BodySetup.Serialize(Ar);
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