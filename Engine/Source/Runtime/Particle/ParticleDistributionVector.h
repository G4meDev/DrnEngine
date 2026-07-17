#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitterInstance;

	enum class EParticleDistributionVectorType : uint8
	{
		Constant,
		Uniform,
		Parameter,
		ConstantCurve,
		Max
	};

	enum class EDistributionVectorParamMode : uint8 
	{
		Normal,
		Abs,
		Direct,
		MAX,
	};

	class ParticleDistributionVector : public RefCountedObject, public Serializable
	{
	public:
		virtual void Serialize(Archive& Ar) override;

		static ParticleDistributionVector* Create(Archive& Ar);
		static ParticleDistributionVector* Create(EParticleDistributionVectorType Type);

		virtual Vector GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) = 0;
		inline virtual EParticleDistributionVectorType GetType() const = 0;

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel);
#endif
	};

	class ParticleDistributionVectorConstant : public ParticleDistributionVector
	{
	public:
		ParticleDistributionVectorConstant()
			: Constant(Vector::ZeroVector)
		{}

		ParticleDistributionVectorConstant(const Vector& InValue)
			: Constant(InValue)
		{}

		Vector Constant;

		virtual void Serialize(Archive& Ar) override;
		virtual Vector GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionVectorType GetType() const override { return EParticleDistributionVectorType::Constant; };

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel) override;
#endif
	};

	class ParticleDistributionVectorUniform : public ParticleDistributionVector
	{
	public:
		ParticleDistributionVectorUniform()
			: Min(Vector::ZeroVector)
			, Max(Vector::OneVector)
		{}

		Vector Min;
		Vector Max;

		virtual void Serialize(Archive& Ar) override;
		virtual Vector GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionVectorType GetType() const override { return EParticleDistributionVectorType::Uniform; };

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel) override;
#endif
	};

	class ParticleDistributionVectorParameter : public ParticleDistributionVector
	{
	public:
		ParticleDistributionVectorParameter()
			: MinInput(Vector::ZeroVector)
			, MaxInput(Vector::OneVector)
			, MinOutput(Vector::ZeroVector)
			, MaxOutput(Vector::OneVector)
			, Constant(Vector::OneVector)
			, ParameterName("None")
			, ParamModes{EDistributionVectorParamMode::Normal, EDistributionVectorParamMode::Normal, EDistributionVectorParamMode::Normal}
		{}

		Vector MinInput;
		Vector MaxInput;
		Vector MinOutput;
		Vector MaxOutput;
		Vector Constant;

		std::string ParameterName;
		EDistributionVectorParamMode ParamModes[3];

		virtual void Serialize(Archive& Ar) override;
		virtual Vector GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionVectorType GetType() const override { return EParticleDistributionVectorType::Parameter; };

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel) override;
#endif
	};

	class ParticleDistributionVectorConstantCurve : public ParticleDistributionVector
	{
	public:
		ParticleDistributionVectorConstantCurve()
			: ConstantCurve()
		{}

		InterpCurveVector ConstantCurve;

		virtual void Serialize(Archive& Ar) override;
		virtual Vector GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionVectorType GetType() const override { return EParticleDistributionVectorType::ConstantCurve; };

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel) override;
#endif
	};
}