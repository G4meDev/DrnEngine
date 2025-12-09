#include "DrnPCH.h"
#include "TextureCube.h"
#include "Editor/AssetImporter/AssetImporterTexture.h"
#include "Editor/AssetPreview/AssetPreviewTextureCubeGuiLayer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	TextureCube::TextureCube( const std::string& InPath )
		: Texture(InPath)
	{
		Load();
	}

#if WITH_EDITOR
	TextureCube::TextureCube( const std::string& InPath, const std::string& InSourcePath )
		: Texture(InPath)
	{
		m_SourcePath = InSourcePath;

		Import();
		Save();
	}
#endif

	TextureCube::~TextureCube()
	{
		ReleaseDescriptors();
	}

	void TextureCube::Serialize( Archive& Ar )
	{
		Texture::Serialize(Ar);

		if (Ar.IsLoading())
		{
			
		}
		else
		{

		}
	}

	void TextureCube::InitResources( D3D12CommandList* CommandList )
	{
		m_Initialized = true;
	}

	void TextureCube::UploadResources( D3D12CommandList* CommandList )
	{
		if (!m_Initialized)
		{
			InitResources(CommandList);
		}

		if (IsRenderStateDirty())
		{
			ReleaseResources();
			ReleaseDescriptors();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			std::string ResourceName;
#if D3D12_Debug_INFO
			std::string TextureName = Path::ConvertShortPath(m_Path);
			TextureName = Path::RemoveFileExtension(TextureName);
			ResourceName = "TextureCube_" + TextureName;
#endif

			RenderResourceCreateInfo TextureCreateInfo( m_ImageBlob->GetBufferPointer(), nullptr, ClearValueBinding::Black, ResourceName );
			m_RenderTexture = RenderTextureCube::Create(CommandList, m_SizeX, m_Format, m_MipLevels, 1, false,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource), TextureCreateInfo);

			// TODO: improve / remove
			CommandList->AddTransitionBarrier(m_RenderTexture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			CommandList->FlushBarriers();

			ClearRenderStateDirty();
		}

	}

	void TextureCube::ReleaseDescriptors()
	{
		
	}

	uint32 TextureCube::GetTextureIndex() const
	{
		drn_check(m_RenderTexture);
		return m_RenderTexture->GetShaderResourceView()->GetDescriptorHeapIndex();
	}

#if WITH_EDITOR
	void TextureCube::Import()
	{
		AssetImporterTexture::Import(this, m_SourcePath);
		Save();
		//Load();

		MarkRenderStateDirty();
	}

	void TextureCube::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewTextureCubeGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void TextureCube::CloseAssetPreview()
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