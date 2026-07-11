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
		bool IsUsingTemplate(AssetHandle<ParticleSystem> InTemplate);
		void InitParticles();

		bool HasCompleted();
		void Complete();
		void ResetParticles( bool bEmptyInstances = false );

		bool ShouldActivate();

		void Deactivate();
		void DeactivateSystem();

		void Activate();
		void ActivateSystem();


#if WITH_EDITOR
		virtual void DrawDetailPanel(float DeltaTime) override;

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
		inline virtual bool HasSprite() const override { return true; }
#endif

	private:
		std::vector<TRefCountPtr<ParticleEmitterInstance>> Emitters;
		AssetHandle<ParticleSystem> Template;

		bool bWasCompleted;
		bool bWasDeactivated;
		bool bSuppressSpawning;
		bool bDeactivateTriggered;
		bool bWasActive;

		int32 TotalActiveParticles;
		uint32 NumSignificantEmitters;

	};
}