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
		, m_RenderStateDirty(true)
	{
		Load();
	}

#if WITH_EDITOR
	StaticMesh::StaticMesh( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
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

	void StaticMesh::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{

	}

	void StaticMesh::UploadResources( ID3D12GraphicsCommandList2* CommandList )
	{
		if (!IsRenderStateDirty())
			return;

		for (int i = 0; i < Data.MeshesData.size(); i++)
		{
			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
			StaticMeshSlotData& Proxy = Data.MeshesData[i];

			std::string MeshName = "";
#if D3D12_Debug_INFO
			MeshName = m_Path;
			MeshName = Path::ConvertShortPath(MeshName);
			MeshName = Path::RemoveFileExtension(MeshName);
#endif

			Proxy.ReleaseBuffers();

			uint32 Stride = sizeof(InputLayout_StaticMesh);
			Proxy.m_VertexBuffer = VertexBuffer::Create(CommandList, Proxy.VertexBufferBlob->GetBufferPointer(),
				Proxy.VertexBufferBlob->GetBufferSize() / Stride, Stride, MeshName);

			uint32 IndexCount = Proxy.IndexBufferBlob->GetBufferSize() / sizeof(uint32);
			Proxy.m_IndexBuffer = IndexBuffer::Create(CommandList, Proxy.IndexBufferBlob->GetBufferPointer(),
				IndexCount, Proxy.IndexBufferBlob->GetBufferSize(), DXGI_FORMAT_R32_UINT, MeshName);
		}

		ClearRenderStateDirty();
	}

	AssetHandle<Material> StaticMesh::GetMaterialAtIndex( uint32 Index )
	{
		if (Index >= 0 && Index < Data.Materials.size())
		{
			AssetHandle<Material> Result;
			Result = Data.Materials[Index].m_Material;
			return Result;
		}

		return AssetHandle<Material>( "InvalidPath" );
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

		MarkRenderStateDirty();
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
				MeshesData[i].Serialize(Ar);
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

			uint64 VertexCount;
			Ar >> VertexCount;

			Positions.resize(VertexCount);
			Normals.resize(VertexCount);
			Tangents.resize(VertexCount);
			BitTangents.resize(VertexCount);

			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar >> Positions[i]; }
			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar >> Normals[i]; }
			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar >> Tangents[i]; }
			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar >> BitTangents[i]; }
		}
		else
		{
			Ar << VertexBufferBlob;
			Ar << IndexBufferBlob;
			Ar << MaterialIndex;

			const uint64 VertexCount = Positions.size();
			Ar << VertexCount;

			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar << Positions[i]; }
			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar << Normals[i]; }
			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar << Tangents[i]; }
			for ( uint64 i = 0; i < VertexCount; i++ ) { Ar << BitTangents[i]; }
		}
	}


	void StaticMeshSlotData::ReleaseBuffers()
	{
		if (m_VertexBuffer) 
		{
			delete m_VertexBuffer;
			m_VertexBuffer = nullptr;
		}
		if (m_IndexBuffer)
		{
			delete m_IndexBuffer;
			m_IndexBuffer = nullptr;
		}
	}

	void StaticMeshSlotData::BindAndDraw( ID3D12GraphicsCommandList2* CommandList ) const
	{
		m_VertexBuffer->Bind(CommandList);
		m_IndexBuffer->Bind(CommandList);
		CommandList->DrawIndexedInstanced(m_IndexBuffer->m_IndexCount, 1, 0, 0, 0);
	}

	StaticMeshSlotData::StaticMeshSlotData()
		: m_VertexBuffer(nullptr)
		, m_IndexBuffer(nullptr)
	{
		
	}

	StaticMeshSlotData::~StaticMeshSlotData()
	{
		ReleaseBlobs();
		ReleaseBuffers();
	}
}