#pragma once

#include "ForwardTypes.h"
#include "Texture.h"
#include "Runtime/Renderer/ResourceView.h"

namespace Drn
{
	class AssetPreviewTexture2DGuiLayer;

	class Texture2D : public Texture
	{
	public:
		Texture2D(const std::string& InPath);
#if WITH_EDITOR
		Texture2D(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual ~Texture2D();

		virtual void Serialize( Archive& Ar ) override;

		void InitResources( ID3D12GraphicsCommandList2* CommandList );
		void UploadResources( ID3D12GraphicsCommandList2* CommandList );

		EAssetType GetAssetType() override { return EAssetType::Texture2D; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Texture2D; }

		inline uint32 GetTextureIndex() const { return m_DescriptorHandle.GetIndex(); }
		inline const DescriptorHandleSRV& GetDescriptorHandle() const { return m_DescriptorHandle; }

	protected:

		DescriptorHandleSRV m_DescriptorHandle;
		bool m_Initialized = false;

#if WITH_EDITOR
		void Import();

		void OpenAssetPreview() override;
		void CloseAssetPreview() override;

		AssetPreviewTexture2DGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewTexture2DGuiLayer;
	};
}