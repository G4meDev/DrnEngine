#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

LOG_DECLARE_CATEGORY(LogAssetPreview);

namespace Drn
{
	class AssetPreview : public Serializable
	{
	public:
		AssetPreview(const std::string InPath);
		AssetPreview(const std::string& InPath, const std::string InSourcePath);
		virtual ~AssetPreview();

		static std::shared_ptr<AssetPreview> Open(const std::string InPath);
		static void Create(const std::string& SourceFilePath, const std::string& TargetDirectoryPath);

		inline std::string GetPath() { return m_Path; }

		virtual void SetCurrentFocus() = 0;
		virtual void Reimport() = 0;

		virtual EAssetType GetAssetType() = 0;

		virtual void Serialize(Archive& Ar) override;

		inline std::string GetSourcePath() { return m_SourcePath; }

	protected:
		std::string m_Path;
		std::string m_SourcePath;

	private:
	};
}

#endif