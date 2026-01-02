#pragma once

#include "ForwardTypes.h"
#include "Texture.h"

namespace Drn
{
	class TextureCube : public Texture
	{
	public:
		TextureCube(const std::string& InPath);
#if WITH_EDITOR
		TextureCube(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual ~TextureCube();

		virtual void Serialize( Archive& Ar ) override;

		void InitResources( class D3D12CommandList* CommandList );
		void UploadResources( class D3D12CommandList* CommandList );

		EAssetType GetAssetType() override { return EAssetType::TextureCube; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::TextureCube; }
	
		virtual uint32 GetTextureIndex() const override;
		virtual uint32 GetSamplerIndex() const override;

		const TRefCountPtr<RenderTextureCube>& GetRenderTexture() const { return m_RenderTexture; }

	protected:

		bool m_Initialized = false;
		TRefCountPtr<class RenderTextureCube> m_RenderTexture;
		TRefCountPtr<SamplerState> m_SamplerState;

		EFilteringMethod FilteringMethod;
		uint8 LODBias;

	public:

#if WITH_EDITOR
		void Import();

		void OpenAssetPreview() override;
		void CloseAssetPreview() override;

		class AssetPreviewTextureCubeGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewTextureCubeGuiLayer;
		friend class AssetImporterTexture;
	};
}