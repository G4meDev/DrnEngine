#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"
#include "Runtime/Particle/ParticleEmitterInstance.h"

namespace Drn
{
	class ParticleEmitterInstance;
	class ParticleSystem;

	class ParticleSystemComponent : public SceneComponent
	{
	public:
		ParticleSystemComponent();
		virtual ~ParticleSystemComponent();

		virtual void Tick(float DeltaTime) override;
		virtual void Serialize( Archive& Ar ) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::ParticleSystemComponent; }

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		void SetTemplate(AssetHandle<ParticleSystem> InTemplate);
		void ResetEmitters();

		float TestParam = 0;

#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
		inline virtual bool HasSprite() const override { return true; }
#endif

	private:
		std::vector<TRefCountPtr<ParticleEmitterInstance>> Emitters;
		AssetHandle<ParticleSystem> Template;

	};
}