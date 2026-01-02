#include "DrnPCH.h"
#include "TextureCube.h"
#include "Editor/AssetImporter/AssetImporterTexture.h"
#include "Editor/AssetPreview/AssetPreviewTextureCubeGuiLayer.h"
#include "Runtime/Renderer/RenderTexture.h"

namespace Drn
{
	TextureCube::TextureCube( const std::string& InPath )
		: Texture(InPath)
		, FilteringMethod(EFilteringMethod::Anisotropic)
		, LODBias(0)
	{
		Load();
	}

#if WITH_EDITOR
	TextureCube::TextureCube( const std::string& InPath, const std::string& InSourcePath )
		: Texture(InPath)
		, FilteringMethod(EFilteringMethod::Anisotropic)
		, LODBias(0)
	{
		m_SourcePath = InSourcePath;

		Import();
		Save();
	}
#endif

	TextureCube::~TextureCube()
	{}

	void TextureCube::Serialize( Archive& Ar )
	{
		Texture::Serialize(Ar);

		if (Ar.IsLoading())
		{
			uint8 PackedState = 0;
			Ar >> PackedState;
			FilteringMethod = (EFilteringMethod)(PackedState & 0x3); // first 2 bit
			LODBias = ((PackedState >> 2) & 0xF); // next 4 bit
		}
		else
		{
			uint8 PackedState = 0;
			PackedState |= (uint8)FilteringMethod;
			PackedState |= (uint8)LODBias << 2;

			Ar << PackedState;
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
			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			std::string ResourceName;
#if D3D12_Debug_INFO
			std::string TextureName = Path::ConvertShortPath(m_Path);
			TextureName = Path::RemoveFileExtension(TextureName);
			ResourceName = "TextureCube_" + TextureName;
#endif

			RenderResourceCreateInfo TextureCreateInfo( m_ImageBlob->GetBufferPointer(), nullptr, ClearValueBinding::Black, ResourceName );
			m_RenderTexture = RenderTextureCube::Create(CommandList, m_SizeX, m_Format, m_MipLevels, 1, false,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::NoFastClear), TextureCreateInfo);

			SamplerStateInitializer SamplerInit((ESamplerFilter)FilteringMethod, ESamplerAddressMode::Wrap, ESamplerAddressMode::Wrap, ESamplerAddressMode::Wrap, LODBias, 0, 0.0f, FLT_MAX, Color::Black, ESamplerCompareFunction::Never);
			m_SamplerState = SamplerState::Create(CommandList->GetParentDevice(), SamplerInit);

			ClearRenderStateDirty();
		}

	}

	uint32 TextureCube::GetTextureIndex() const
	{
		drn_check(m_RenderTexture);
		return m_RenderTexture->GetShaderResourceView()->GetDescriptorHeapIndex();
	}

	uint32 TextureCube::GetSamplerIndex() const
	{
		drn_check(m_SamplerState);
		return m_SamplerState->GetIndex();
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