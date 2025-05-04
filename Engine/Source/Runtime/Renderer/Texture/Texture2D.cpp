#include "DrnPCH.h"
#include "Texture2D.h"

#include "Editor/AssetImporter/AssetImporterTexture.h"
#include "Editor/AssetPreview/AssetPreviewTexture2DGuiLayer.h"

namespace Drn
{
	Texture2D::Texture2D( const std::string& InPath )
		: Texture(InPath)
	{
		Load();
	}

#if WITH_EDITOR
	Texture2D::Texture2D( const std::string& InPath, const std::string& InSourcePath )
		: Texture(InPath)
	{
		m_SourcePath = InSourcePath;

		Import();
		Save();
	}
#endif
 
	Texture2D::~Texture2D()
	{
		
	}

	void Texture2D::Serialize( Archive& Ar )
	{
		Texture::Serialize(Ar);

		if (Ar.IsLoading())
		{
			
		}

		else
		{

		}
	}

	void Texture2D::UploadResources( dx12lib::CommandList* CommandList )
	{
		// TODO: release intermediate resource

		if (IsRenderStateDirty())
		{
			ReleaseResources();

			D3D12_RESOURCE_DESC TextureDesc = {};
			TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			TextureDesc.Width = m_SizeX;
			TextureDesc.Height = m_SizeY;
			TextureDesc.DepthOrArraySize = 1;
			TextureDesc.MipLevels = m_MipLevels;
			TextureDesc.Format = m_Format;
			TextureDesc.SampleDesc.Count = 1;
			TextureDesc.SampleDesc.Quality = 0;
			TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			TextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			D3D12_HEAP_PROPERTIES HeapProps;
			HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			HeapProps.CreationNodeMask = 1;
			HeapProps.VisibleNodeMask = 1;

			CommandList->GetDevice().GetD3D12Device()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &TextureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Resource));

#if D3D12_Debug_INFO
			std::string TextureName = Path::ConvertShortPath(m_Path);
			TextureName = Path::RemoveFileExtension(TextureName);
			m_Resource->SetName(StringHelper::s2ws(TextureName).c_str());
#endif

			D3D12_SUBRESOURCE_DATA TextureResource;
			TextureResource.pData = m_ImageBlob->GetBufferPointer();
			TextureResource.RowPitch = m_RowPitch;
			TextureResource.SlicePitch = m_SlicePitch;

			uint64 UploadBufferSize = GetRequiredIntermediateSize(m_Resource, 0, 1);

			CommandList->GetDevice().GetD3D12Device()->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
				D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer( UploadBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				IID_PPV_ARGS( &m_IntermediateResource ) );

			UpdateSubresources(CommandList->GetD3D12CommandList().Get(), m_Resource, m_IntermediateResource, 0, 0, 1, &TextureResource);

			ClearRenderStateDirty();
		}
	}

#if WITH_EDITOR
	void Texture2D::Import()
	{
		AssetImporterTexture::Import(this, m_SourcePath);
		Save();
		Load();

		MarkRenderStateDirty();
	}

	void Texture2D::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewTexture2DGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void Texture2D::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}
#endif
}