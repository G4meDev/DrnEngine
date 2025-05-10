#pragma once

#include "ForwardTypes.h"
#include "Texture.h"

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

		void UploadResources( ID3D12GraphicsCommandList2* CommandList );

		D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;

	protected:

		EAssetType GetAssetType() override { return EAssetType::Texture2D; };
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::Texture2D; }

#if WITH_EDITOR
		void Import();

		void OpenAssetPreview() override;
		void CloseAssetPreview() override;

		AssetPreviewTexture2DGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewTexture2DGuiLayer;
	};
}