#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SceneComponent.h"
#include "Runtime/Particle/ParticleEmitterInstance.h"

namespace Drn
{
	class ParticleEmitterInstance;
	class ParticleSystem;

	//enum class EParticleSysParamType : uint8
	//{
	//	PSPT_None,
	//	PSPT_Scalar,
	//	PSPT_ScalarRand,
	//	PSPT_Vector,
	//	PSPT_VectorRand,
	//	PSPT_MAX,
	//};

	struct ParticleSysParam : public Serializable
	{
		ParticleSysParam() : Name("None") {};

		virtual void Serialize(Archive& Ar) override;

		std::string Name;

#if WITH_EDITOR
		virtual bool Draw();
#endif
	};

	struct ParticleSysParamFloat : ParticleSysParam
	{
		ParticleSysParamFloat()
			: ParticleSysParam()
			, bUseLowRange(false)
			, Scalar(1.0f)
			, Scalar_Low(0.0f)
		{}

		virtual void Serialize(Archive& Ar) override;

		bool bUseLowRange;
		float Scalar;
		float Scalar_Low;

#if WITH_EDITOR
		virtual bool Draw() override;
#endif
	};

	struct ParticleSysParamVector : ParticleSysParam
	{
		ParticleSysParamVector()
			: ParticleSysParam()
			, bUseLowRange(false)
			, Value(Vector::ZeroVector)
			, Value_Low(Vector::ZeroVector)
		{}

		virtual void Serialize(Archive& Ar) override;

		bool bUseLowRange;
		Vector Value;
		Vector Value_Low;

#if WITH_EDITOR
		virtual bool Draw() override;
#endif
	};

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

		virtual BoxSphereBounds CalcBounds( const Transform& LocalToWorld ) const override;

		void SetFloatParameter( const std::string& InName, const ParticleSysParamFloat& InParam );
		bool GetFloatParameter( const std::string& InName, float& OutFloat );

		void SetVectorParameter( const std::string& InName, const ParticleSysParamVector& InParam );
		bool GetVectorParameter( const std::string& InName, Vector& OutVector );

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
		bool bWarmingUp;

		int32 TotalActiveParticles;
		uint32 NumSignificantEmitters;

		RandomStream RandStream;

		std::vector<ParticleSysParamFloat> FloatParams;
		std::vector<ParticleSysParamVector> VectorParams;

		friend class ParticleEmitterInstance;
		friend class ParticleMeshEmitterInstance;
		friend class AssetPreviewParticleSystemGuiLayer;
	};
}