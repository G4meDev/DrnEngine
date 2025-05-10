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

			//D3D12_RESOURCE_STATE_GENERIC_READ;

			Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer( Proxy.VertexBufferBlob->GetBufferSize() ),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( Proxy.IntermediateVertexBuffer.ReleaseAndGetAddressOf() ) );

			Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer( Proxy.VertexBufferBlob->GetBufferSize() ),
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS( Proxy.VertexBuffer.ReleaseAndGetAddressOf() ) );

			Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer( Proxy.VertexBufferBlob->GetBufferSize() ),
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS( Proxy.IntermediateIndexBuffer.ReleaseAndGetAddressOf() ) );

			Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer( Proxy.VertexBufferBlob->GetBufferSize() ),
				D3D12_RESOURCE_STATE_INDEX_BUFFER, nullptr, IID_PPV_ARGS( Proxy.IndexBuffer.ReleaseAndGetAddressOf() ) );

#if D3D12_Debug_INFO
			std::string MeshName = m_Path;
			MeshName = Path::ConvertShortPath(MeshName);
			MeshName = Path::RemoveFileExtension(MeshName);

			Proxy.IntermediateVertexBuffer->SetName( StringHelper::s2ws(std::string("IntermediateVertexBuffer_") + MeshName).c_str() );
			Proxy.VertexBuffer->SetName( StringHelper::s2ws(std::string("VertexBuffer_") + MeshName).c_str() );
			Proxy.IntermediateIndexBuffer->SetName( StringHelper::s2ws(std::string("IntermediateIndexBuffer_") + MeshName).c_str() );
			Proxy.IndexBuffer->SetName( StringHelper::s2ws(std::string("IndexBuffer_") + MeshName).c_str() );
#endif

			{
				UINT8*        pVertexDataBegin;
				CD3DX12_RANGE readRange( 0, 0 );
				Proxy.IntermediateVertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) );
				uint32 ByteSize = Proxy.VertexBufferBlob->GetBufferSize();
				memcpy( pVertexDataBegin, Proxy.VertexBufferBlob->GetBufferPointer(), ByteSize );
				Proxy.IntermediateVertexBuffer->Unmap( 0, nullptr );

				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					Proxy.VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST );
				CommandList->ResourceBarrier(1, &barrier);

				CommandList->CopyResource(Proxy.VertexBuffer.Get(), Proxy.IntermediateVertexBuffer.Get());

				barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					Proxy.VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );
				CommandList->ResourceBarrier(1, &barrier);

				Proxy.m_VertexBufferView.BufferLocation = Proxy.VertexBuffer->GetGPUVirtualAddress();
				Proxy.m_VertexBufferView.StrideInBytes  = sizeof( InputLayout_StaticMesh );
				Proxy.m_VertexBufferView.SizeInBytes    = ByteSize;
			}

			{
				UINT8*        pIndexDataBegin;
				CD3DX12_RANGE readRange( 0, 0 );
				Proxy.IntermediateIndexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pIndexDataBegin ) );
				uint32 ByteSize = Proxy.IndexBufferBlob->GetBufferSize();
				memcpy( pIndexDataBegin, Proxy.IndexBufferBlob->GetBufferPointer(), ByteSize );
				Proxy.IntermediateIndexBuffer->Unmap( 0, nullptr );

				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					Proxy.IndexBuffer.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST );
				CommandList->ResourceBarrier(1, &barrier);

				CommandList->CopyResource(Proxy.IndexBuffer.Get(), Proxy.IntermediateIndexBuffer.Get());

				barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					Proxy.IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER );
				CommandList->ResourceBarrier(1, &barrier);

				Proxy.m_IndexBufferView.BufferLocation = Proxy.IndexBuffer->GetGPUVirtualAddress();
				Proxy.m_IndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
				Proxy.m_IndexBufferView.SizeInBytes    = ByteSize;
			}

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
		}
		else
		{
			Ar << VertexBufferBlob;
			Ar << IndexBufferBlob;
			Ar << MaterialIndex;
		}
	}


	StaticMeshSlotData::~StaticMeshSlotData()
	{
		ReleaseBlobs();
	}
}