#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "AssetManager.h"

LOG_DECLARE_CATEGORY( LogAsset )

namespace Drn
{
	enum class EAssetType : uint16
	{
		Undefined = 0,
		StaticMesh,
		Level,
		Material,
		Texture2D,
		TextureVolume,
		TextureCube
	};

	class Asset : public Serializable
	{
	public:
		Asset(const std::string InPath);
		virtual ~Asset() = default;

		uint16 m_RefCount;

		inline void AddRef() { m_RefCount++; }
		inline void RemoveRef() { m_RefCount--; }

		virtual void Serialize( Archive& Ar ) override;

	protected:

		virtual void Load();

		virtual EAssetType GetAssetType() = 0;

#if WITH_EDITOR
		virtual void Save();

		virtual void OpenAssetPreview() = 0;
		virtual void CloseAssetPreview() = 0;
#endif

		uint16 m_AssetType;
		std::string m_Path;

		friend class AssetManager;
		friend class Editor;
	private:

	};
}