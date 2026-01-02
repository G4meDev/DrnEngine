#include "DrnPCH.h"
#include "Texture2D.h"

#include "Editor/AssetImporter/AssetImporterTexture.h"
#include "Editor/AssetPreview/AssetPreviewTexture2DGuiLayer.h"

namespace Drn
{
	Texture2D::Texture2D( const std::string& InPath )
		: Texture(InPath)
		, FilteringMethod(EFilteringMethod::Trilinear)
		, TilingMethodX(ETilingMethod::Wrap)
		, TilingMethodY(ETilingMethod::Wrap)
		, LODBias(0)
	{
		Load();
	}

#if WITH_EDITOR
	Texture2D::Texture2D( const std::string& InPath, const std::string& InSourcePath )
		: Texture(InPath)
		, FilteringMethod(EFilteringMethod::Trilinear)
		, TilingMethodX(ETilingMethod::Wrap)
		, TilingMethodY(ETilingMethod::Wrap)
		, LODBias(0)
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
			uint8 PackedState = 0;
			Ar >> PackedState;
			FilteringMethod = (EFilteringMethod)(PackedState & 0x3);
			TilingMethodX = (ETilingMethod)((PackedState >> 2) & 0x3);
			TilingMethodY = (ETilingMethod)((PackedState >> 4) & 0x3);

			Ar >> LODBias;
		}

		else
		{
			uint8 PackedState = 0;
			PackedState |= (uint8)FilteringMethod;
			PackedState |= (uint8)TilingMethodX << 2;
			PackedState |= (uint8)TilingMethodY << 4;

			Ar << PackedState;
			Ar << LODBias;
		}
	}

	void Texture2D::InitResources( D3D12CommandList* CommandList )
	{
		m_Initialized = true;
	}

	void Texture2D::UploadResources( D3D12CommandList* CommandList )
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
			ResourceName = "Texture2D_" + TextureName;
#endif

			RenderResourceCreateInfo TextureCreateInfo( m_ImageBlob->GetBufferPointer(), nullptr, ClearValueBinding::Black, ResourceName );
			m_RenderTexture = RenderTexture2D::Create(CommandList, m_SizeX, m_SizeY, m_Format, m_MipLevels, 1, false,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::NoFastClear), TextureCreateInfo);

			SamplerStateInitializer SamplerInit((ESamplerFilter)FilteringMethod, ESamplerAddressMode(TilingMethodX), ESamplerAddressMode(TilingMethodY), ESamplerAddressMode(TilingMethodX), LODBias, 0, 0.0f, FLT_MAX, Color::Black, ESamplerCompareFunction::Never);
			m_SamplerState = SamplerState::Create(CommandList->GetParentDevice(), SamplerInit);

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
	}

	uint32 Texture2D::GetSamplerIndex() const
	{
		drn_check(m_SamplerState);
		return m_SamplerState->GetIndex();
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