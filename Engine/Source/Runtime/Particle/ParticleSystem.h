#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"

namespace Drn
{
	class ParticleEmitter;

	class ParticleSystem : public Asset
	{
	public:
		ParticleSystem(const std::string& Path);
		virtual ~ParticleSystem();

#if WITH_EDITOR
		ParticleSystem(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual void Serialize(Archive& Ar) override;

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::ParticleSystem; }

		std::vector<TRefCountPtr<ParticleEmitter>> Emitters;

		bool bAutoDeactivate;

		bool bUseFixedBounds;
		Vector FixedBoundsMin;
		Vector FixedBoundsMax;

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		bool Draw();

		class AssetPreviewParticleSystemGuiLayer* GuiLayer = nullptr;
#endif

	private:

		friend class AssetPreviewParticleSystemGuiLayer;
	};
}