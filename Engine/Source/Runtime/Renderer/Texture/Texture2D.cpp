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
		m_DescriptorHandle.FreeDescriptorSlot();
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

	void Texture2D::InitResources( D3D12CommandList* CommandList )
	{
		m_Initialized = true;

		m_DescriptorHandle.AllocateDescriptorSlot();
	}

	void Texture2D::UploadResources( D3D12CommandList* CommandList )
	{
		if (!m_Initialized)
		{
			InitResources(CommandList);
		}

		if (IsRenderStateDirty())
		{
			ReleaseResources();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			std::string ResourceName;
#if D3D12_Debug_INFO
			std::string TextureName = Path::ConvertShortPath(m_Path);
			TextureName = Path::RemoveFileExtension(TextureName);
			ResourceName = "Texture2D_" + TextureName;
#endif

			RenderResourceCreateInfo TextureCreateInfo( m_ImageBlob->GetBufferPointer(), nullptr, ClearValueBinding::Black, ResourceName );
			m_RenderTexture = RenderTexture2D::Create(CommandList, m_SizeX, m_SizeY, m_Format, m_MipLevels, 1, false,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource), TextureCreateInfo);

			// TODO: improve / remove
			CommandList->AddTransitionBarrier(m_RenderTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			CommandList->FlushBarriers();

			const uint32 SubresourceCount = m_MipLevels;

			CD3DX12_RESOURCE_DESC TextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_Format, m_SizeX, m_SizeY, 1, m_MipLevels, 1, 0);
			m_Resource = Resource::Create(D3D12_HEAP_TYPE_DEFAULT, TextureDesc, D3D12_RESOURCE_STATE_COPY_DEST);

			uint64 TextureMemorySize = 0;
			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> Layouts;
			Layouts.resize(SubresourceCount);

			std::vector<uint32> NumRows;
			NumRows.resize(SubresourceCount);

			std::vector<uint64> RowSizeInBytes;
			RowSizeInBytes.resize(SubresourceCount);

			Device->GetCopyableFootprints(&TextureDesc, 0, SubresourceCount, 0, Layouts.data(), NumRows.data(), RowSizeInBytes.data(), &TextureMemorySize);

			std::vector<D3D12_SUBRESOURCE_DATA> TextureResource;
			TextureResource.resize(SubresourceCount);
			for (int32 i = 0; i < SubresourceCount; i++)
			{
				TextureResource[i].pData = (BYTE*)m_ImageBlob->GetBufferPointer() + Layouts[i].Offset;
				TextureResource[i].RowPitch = Layouts[i].Footprint.RowPitch;
				TextureResource[i].SlicePitch = Layouts[i].Footprint.RowPitch * NumRows[i];
			}

			Resource* IntermediateResource = Resource::Create(D3D12_HEAP_TYPE_UPLOAD,
				CD3DX12_RESOURCE_DESC::Buffer( TextureMemorySize ), D3D12_RESOURCE_STATE_GENERIC_READ, false);

			// TODO: maybe 1 frame lifetime is enough instead of NUM_BACKBUFFER
			IntermediateResource->ReleaseBufferedResource();

//#if D3D12_Debug_INFO
//			std::string TextureName = Path::ConvertShortPath(m_Path);
//			TextureName = Path::RemoveFileExtension(TextureName);
//			m_Resource->SetName("TextureResource_" + TextureName);
//			IntermediateResource->SetName("TextureIntermediateResource_" + TextureName);
//#endif

			UpdateSubresources(CommandList->GetD3D12CommandList(), m_Resource->GetD3D12Resource(),
				IntermediateResource->GetD3D12Resource(), 0, SubresourceCount, TextureMemorySize, Layouts.data(), NumRows.data(), RowSizeInBytes.data(), TextureResource.data());

			ResourceStateTracker::Get()->TransiationResource(m_Resource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList->GetD3D12CommandList());

			D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc = {};
			ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			ResourceViewDesc.Format = m_Format;
			ResourceViewDesc.Texture2D.MipLevels = m_MipLevels;
			ResourceViewDesc.Texture2D.MostDetailedMip = 0;

			m_DescriptorHandle.CreateView(ResourceViewDesc, m_Resource->GetD3D12Resource());

// -----------------------------------------------------------------------------------------------------------------------------

			//Renderer::Get()->TempSamplerAllocator.Alloc(&SamplerCpuHandle, &SamplerGpuHandle);
			//
			//D3D12_SAMPLER_DESC SamplerDesc = {};
			//SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			//SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			//SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			//SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
			//SamplerDesc.MaxAnisotropy = 16;
			//Device->CreateSampler(&SamplerDesc, SamplerCpuHandle);

			ClearRenderStateDirty();
		}
	}

	RenderTexture2D* Texture2D::GetRenderTexture()
	{
		return m_RenderTexture;
	}

	uint32 Texture2D::GetTextureIndex() const
	{
		drn_check(m_RenderTexture);

		return m_RenderTexture->GetShaderResourceView()->GetDescriptorHeapIndex();
		//return m_DescriptorHandle.GetIndex();
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