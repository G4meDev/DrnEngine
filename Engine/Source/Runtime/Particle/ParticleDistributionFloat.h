#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitterInstance;

	enum class EParticleDistributionFloatType : uint8
	{
		Constant,
		Uniform,
		Parameter,
		Max
	};

	enum class EDistributionFloatParamMode : uint8 
	{
		Normal,
		Abs,
		Direct,
		MAX,
	};

	class ParticleDistributionFloat : public RefCountedObject, public Serializable
	{
	public:
		virtual void Serialize(Archive& Ar) override;

		static ParticleDistributionFloat* Create(Archive& Ar);
		static ParticleDistributionFloat* Create(EParticleDistributionFloatType Type);

		virtual float GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) = 0;
		inline virtual EParticleDistributionFloatType GetType() const = 0;

		virtual void GetOutRange(float& MinOut, float& MaxOut) const = 0;

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel);
#endif
	};

	class ParticleDistributionFloatConstant : public ParticleDistributionFloat
	{
	public:
		ParticleDistributionFloatConstant()
			: Constant(0)
		{}

		ParticleDistributionFloatConstant(float InValue)
			: Constant(InValue)
		{}

		float Constant;

		virtual void Serialize(Archive& Ar) override;
		virtual float GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionFloatType GetType() const override { return EParticleDistributionFloatType::Constant; };

		virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		{
			MinOut = MaxOut = Constant;
		}

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel) override;
#endif
	};

	class ParticleDistributionFloatUniform : public ParticleDistributionFloat
	{
	public:
		ParticleDistributionFloatUniform()
			: Min(0)
			, Max(1)
		{}

		float Min;
		float Max;

		virtual void Serialize(Archive& Ar) override;
		virtual float GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionFloatType GetType() const override { return EParticleDistributionFloatType::Uniform; };

		virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		{
			MinOut = Min;
			MaxOut = Max;
		}

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel) override;
#endif
	};

	class ParticleDistributionFloatParameter : public ParticleDistributionFloat
	{
	public:
		ParticleDistributionFloatParameter()
			: MinInput(0)
			, MaxInput(1)
			, MinOutput(0)
			, MaxOutput(1)
			, Constant(0)
			, ParameterName("None")
			, ParamMode(EDistributionFloatParamMode::Normal)
		{}

		float MinInput;
		float MaxInput;
		float MinOutput;
		float MaxOutput;
		float Constant;

		std::string ParameterName;
		EDistributionFloatParamMode ParamMode;

		virtual void Serialize(Archive& Ar) override;
		virtual float GetValue( float F, ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionFloatType GetType() const override { return EParticleDistributionFloatType::Parameter; };

		virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		{
			MinOut = MinOutput;
			MaxOut = MaxInput;
		}

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel) override;
#endif
	};
}