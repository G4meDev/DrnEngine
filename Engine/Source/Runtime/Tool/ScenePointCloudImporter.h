#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ScenePointCloudImporter : public Actor
	{
	public:
		ScenePointCloudImporter();
		virtual ~ScenePointCloudImporter();

		EActorType GetActorType() override { return EActorType::ScenePointCloudImporter; };
		inline static EActorType GetActorTypeStatic() { return EActorType::ScenePointCloudImporter; };
		void Serialize( Archive& Ar ) override;

#if WITH_EDITOR
		bool DrawDetailPanel() override;

		void Import(const std::string& FilePath);
#endif
	protected:
		std::shared_ptr<SceneComponent> m_RootComponent;

		std::vector<std::string> FilesPath;
	};
}