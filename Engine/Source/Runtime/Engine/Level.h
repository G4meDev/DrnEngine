#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Core/Serializable.h"

LOG_DECLARE_CATEGORY( LogLevel )


namespace Drn
{
	class Level : public Asset
	{
	public:
		Level(const std::string& Path);
		virtual ~Level();

#if WITH_EDITOR
		Level(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual void Serialize(Archive& Ar) override;

		virtual EAssetType GetAssetType() override;

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;
#endif

	protected:
		

	private:

	};
}