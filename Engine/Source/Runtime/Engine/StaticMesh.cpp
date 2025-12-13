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

			BufferArchive BuffAr(0);
			Ar >> BuffAr;
			BuffAr.Decompress();
			Data.Serialize(BuffAr);

			BuffAr >> ImportScale;
			
			m_BodySetup.Serialize(BuffAr);

			{
				Ar >> m_ImportNormals;
				Ar >> m_ImportTangents;
				Ar >> m_ImportBitTangents;
				Ar >> m_ImportColor;
				Ar >> m_ImportUVs;
			}
		}

#if WITH_EDITOR
		else
		{
			Ar << m_SourcePath;

			BufferArchive BufArr(10, false);
			Data.Serialize( BufArr );

			BufArr << ImportScale;
			m_BodySetup.Serialize(BufArr);

			BufArr.Compress();
			Ar << BufArr;

			{
				Ar << m_ImportNormals;
				Ar << m_ImportTangents;
				Ar << m_ImportBitTangents;
				Ar << m_ImportColor;
				Ar << m_ImportUVs;
			}
		}
#endif
	}

	void StaticMesh::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{

	}

	void StaticMesh::UploadResources( D3D12CommandList* CommandList )
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
			MeshName = Path::RemoveFileExtension(MeshName) + "_" + std::to_string(i);
#endif

			Proxy.ReleaseBuffers();

			uint32 IndexCount = Proxy.VertexData.GetIndices().size();

			uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
			RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, (void*)Proxy.VertexData.GetIndices().data(), ClearValueBinding::Black, "IB" + MeshName);
			Proxy.m_IndexBuffer = RenderIndexBuffer::Create(Renderer::Get()->GetDevice(), CommandList, sizeof(uint32), IndexCount * sizeof(uint32),
				IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);

			Proxy.m_IndexBufferView.BufferLocation = Proxy.m_IndexBuffer->m_ResourceLocation.GetGPUVirtualAddress();
			Proxy.m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
			Proxy.m_IndexBufferView.SizeInBytes = IndexCount * sizeof(uint32);

			Proxy.m_StaticMeshVertexBuffer = StaticMeshVertexBuffer::Create(CommandList->GetD3D12CommandList(), Proxy.VertexData, MeshName);
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
			VertexData.Serialize(Ar);
			Ar >> MaterialIndex;
		}
		else
		{
			VertexData.Serialize(Ar);
			Ar << MaterialIndex;
		}
	}


	void StaticMeshSlotData::ReleaseBuffers()
	{
		if (m_StaticMeshVertexBuffer)
		{
			delete m_StaticMeshVertexBuffer;
			m_StaticMeshVertexBuffer = nullptr;
		}
	}

	void StaticMeshSlotData::BindAndDraw( D3D12CommandList* CommandList ) const
	{
		m_StaticMeshVertexBuffer->Bind(CommandList->GetD3D12CommandList());
		//CommandList->IASetIndexBuffer(&m_IndexBufferView);
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

	StaticMeshSlotData::StaticMeshSlotData()
		: m_IndexBuffer(nullptr)
		, m_StaticMeshVertexBuffer(nullptr)
	{
		
	}

	StaticMeshSlotData::~StaticMeshSlotData()
	{
		ReleaseBuffers();
	}

}