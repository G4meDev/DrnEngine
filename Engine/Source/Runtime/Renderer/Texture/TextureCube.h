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

		void InitResources( ID3D12GraphicsCommandList2* CommandList );
		void UploadResources( ID3D12GraphicsCommandList2* CommandList );

		void ReleaseDescriptors();

		EAssetType GetAssetType() override { return EAssetType::TextureCube; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::TextureCube; }
	
		inline uint32 GetTextureIndex() const { return m_DescriptorHandle.GetIndex(); }

	protected:

		DescriptorHandleSRV m_DescriptorHandle;
		bool m_Initialized = false;

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