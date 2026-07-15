#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitterInstance;

	enum class EParticleDistributionFloatType : uint8
	{
		Constant,
		Uniform,
		Max
	};

	class ParticleDistributionFloat : public RefCountedObject, public Serializable
	{
	public:
		virtual void Serialize(Archive& Ar) override;

		static ParticleDistributionFloat* Create(Archive& Ar);
		static ParticleDistributionFloat* Create(EParticleDistributionFloatType Type);

		virtual float GetValue( ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) = 0;
		inline virtual EParticleDistributionFloatType GetType() const = 0;

		virtual void GetOutRange(float& MinOut, float& MaxOut) const = 0;

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr);
#endif
	};

	class ParticleDistributionFloatConstant : public ParticleDistributionFloat
	{
	public:
		ParticleDistributionFloatConstant()
			: Constant(0)
		{}

		float Constant;

		virtual void Serialize(Archive& Ar) override;
		virtual float GetValue( ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionFloatType GetType() const override { return EParticleDistributionFloatType::Constant; };

		virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		{
			MinOut = MaxOut = Constant;
		}

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr) override;
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
		virtual float GetValue( ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionFloatType GetType() const override { return EParticleDistributionFloatType::Uniform; };

		virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		{
			MinOut = Min;
			MaxOut = Max;
		}

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionFloat>& Ptr) override;
#endif
	};
}