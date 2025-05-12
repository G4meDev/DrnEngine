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
		ReleaseDescriptors();
	}

	void Texture2D::ReleaseDescriptors()
	{
		if (Renderer::Get())
		{
			Renderer::Get()->TempSRVAllocator.Free(TextureCpuHandle, TextureGpuHandle);
			Renderer::Get()->TempSamplerAllocator.Free(SamplerCpuHandle, SamplerGpuHandle);
		}
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

	void Texture2D::UploadResources( ID3D12GraphicsCommandList2* CommandList )
	{
		// TODO: release intermediate resource

		if (IsRenderStateDirty())
		{
			ReleaseResources();
			ReleaseDescriptors();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

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

			m_Resource = Resource::Create(D3D12_HEAP_TYPE_DEFAULT, TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST);

			D3D12_SUBRESOURCE_DATA TextureResource;
			TextureResource.pData = m_ImageBlob->GetBufferPointer();
			TextureResource.RowPitch = m_RowPitch;
			TextureResource.SlicePitch = m_SlicePitch;

			uint64 UploadBufferSize = GetRequiredIntermediateSize(m_Resource->GetD3D12Resource(), 0, 1);

			Resource* IntermediateResource = Resource::Create(D3D12_HEAP_TYPE_UPLOAD,
				CD3DX12_RESOURCE_DESC::Buffer( UploadBufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ);

			// TODO: maybe 1 frame lifetime is enough instead of NUM_BACKBUFFER
			IntermediateResource->ReleaseBufferedResource();

#if D3D12_Debug_INFO
			std::string TextureName = Path::ConvertShortPath(m_Path);
			TextureName = Path::RemoveFileExtension(TextureName);
			m_Resource->SetName("TextureResource_" + TextureName);
			IntermediateResource->SetName("TextureIntermediateResource_" + TextureName);
#endif

			UpdateSubresources(CommandList, m_Resource->GetD3D12Resource(),
				IntermediateResource->GetD3D12Resource(), 0, 0, 1, &TextureResource);

			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_Resource->GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE );
			CommandList->ResourceBarrier(1, &barrier);


			D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			ResourceViewDesc.Format = m_Format;
			ResourceViewDesc.Texture2D.MipLevels = m_MipLevels;
			ResourceViewDesc.Texture2D.MostDetailedMip = 0;

			Renderer::Get()->TempSRVAllocator.Alloc(&TextureCpuHandle, &TextureGpuHandle);
			Device->CreateShaderResourceView(m_Resource->GetD3D12Resource(), &ResourceViewDesc, TextureCpuHandle);

// -----------------------------------------------------------------------------------------------------------------------------

			Renderer::Get()->TempSamplerAllocator.Alloc(&SamplerCpuHandle, &SamplerGpuHandle);

			D3D12_SAMPLER_DESC SamplerDesc = {};
			SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			Device->CreateSampler(&SamplerDesc, SamplerCpuHandle);

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