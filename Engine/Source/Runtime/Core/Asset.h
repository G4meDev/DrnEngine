#pragma once

#include "ForwardTypes.h"
#include "AssetManager.h"

LOG_DECLARE_CATEGORY( LogAsset )

namespace Drn
{
	enum class EAssetType : uint8
	{
		StaticMesh = 0
	};

	class Asset
	{
	public:
		Asset(const std::string InPath);
		Asset(const std::string InPath, const std::string InSourcePath);

		uint16 m_RefCount;

		inline void AddRef() { m_RefCount++; }
		inline void RemoveRef() { m_RefCount--; }

	protected:

		virtual void Load() = 0;

		virtual EAssetType GetAssetType() = 0;

#if WITH_EDITOR
		virtual void Save() = 0;
		virtual void Import() = 0;

		virtual void OpenAssetPreview() = 0;
		virtual void CloseAssetPreview() = 0;
#endif

		uint8 m_AssetType;
		uint8 m_AssetVersion;
		std::string m_SourcePath;
		std::string m_Path;

		friend class AssetManager;
		friend class Editor;
	private:

	};
}