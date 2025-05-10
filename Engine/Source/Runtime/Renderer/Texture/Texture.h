#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"

namespace Drn
{
	class Texture : public Asset
	{
	public:

		Texture(const std::string& InPath)
			: Asset(InPath)
			, m_sRGB(true)
			, m_SizeX(1)
			, m_SizeY(1)
			, m_MipLevels(1)
			, m_Format(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
			, m_RowPitch(1)
			, m_SlicePitch(1)
			, m_ImageBlob(nullptr)
			, m_Resource(nullptr)
			, m_IntermediateResource(nullptr)
			, m_RenderStateDirty(true)
		{
		}

		virtual ~Texture()
		{
			ReleaseImageBlobs();
			ReleaseResources();
		}

		virtual void Serialize( Archive& Ar ) override;

		inline ID3D12Resource* GetResource() { return m_Resource; }

		inline bool IsSRGB() const				{ return m_sRGB; }
		inline uint16 GetSizeX() const			{ return m_SizeX; }
		inline uint16 GetSizeY() const			{ return m_SizeY; }
		inline uint8 GetMipLevels() const		{ return m_MipLevels; }
		inline DXGI_FORMAT GetFormat() const	{ return m_Format; }

		inline bool IsRenderStateDirty() const	{ return m_RenderStateDirty; }
		inline void MarkRenderStateDirty()		{ m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty()		{ m_RenderStateDirty = false; }

		inline void ReleaseImageBlobs()
		{
			if (m_ImageBlob)
			{
				m_ImageBlob->Release();
				m_ImageBlob = nullptr;
			}
		}

		inline void ReleaseResources()
		{
			if (m_Resource)
			{
				m_Resource->Release();
				m_Resource = nullptr;
			}

			if (m_IntermediateResource)
			{
				m_IntermediateResource->Release();
				m_IntermediateResource = nullptr;
			}
		}

protected:
		EAssetType GetAssetType() override = 0;

#if WITH_EDITOR
		void OpenAssetPreview() override = 0;
		void CloseAssetPreview() override = 0;
#endif

		ID3D12Resource* m_Resource;
		//D3D12_Res m_ResourceView;

		// TODO: make single and souble buffer structures to delete this at start of next frame
		ID3D12Resource* m_IntermediateResource;

		bool m_sRGB;
		uint16 m_SizeX;
		uint16 m_SizeY;
		uint8 m_MipLevels;
		DXGI_FORMAT m_Format;

		uint32 m_RowPitch;
		uint32 m_SlicePitch;

		// TODO: make array to support mip levels
		ID3DBlob* m_ImageBlob;

		std::string m_SourcePath;
		bool m_RenderStateDirty;

		friend class AssetImporterTexture;
	};
}