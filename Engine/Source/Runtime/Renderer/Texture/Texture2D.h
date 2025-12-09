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

		void InitResources( class D3D12CommandList* CommandList );
		void UploadResources( class D3D12CommandList* CommandList );

		EAssetType GetAssetType() override { return EAssetType::Texture2D; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Texture2D; }

		RenderTexture2D* GetRenderTexture();
		uint32 GetTextureIndex() const;
		//inline const DescriptorHandleSRV& GetDescriptorHandle() const { return m_DescriptorHandle; }

	protected:

		DescriptorHandleSRV m_DescriptorHandle;
		bool m_Initialized = false;

		TRefCountPtr<class RenderTexture2D> m_RenderTexture;

#if WITH_EDITOR
		void Import();

		void OpenAssetPreview() override;
		void CloseAssetPreview() override;

		AssetPreviewTexture2DGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewTexture2DGuiLayer;
	};
}