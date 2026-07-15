#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitterInstance;

	enum class EParticleDistributionVectorType : uint8
	{
		Constant,
		Uniform,
		Max
	};

	class ParticleDistributionVector : public RefCountedObject, public Serializable
	{
	public:
		virtual void Serialize(Archive& Ar) override;

		static ParticleDistributionVector* Create(Archive& Ar);
		static ParticleDistributionVector* Create(EParticleDistributionVectorType Type);

		virtual Vector GetValue( ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) = 0;
		inline virtual EParticleDistributionVectorType GetType() const = 0;

		//virtual void GetOutRange(float& MinOut, float& MaxOut) const = 0;

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
		virtual Vector GetValue( ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionVectorType GetType() const override { return EParticleDistributionVectorType::Constant; };

		//virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		//{
		//	MinOut = MaxOut = Constant;
		//}

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
		virtual Vector GetValue( ParticleEmitterInstance* Emitter = nullptr, RandomStream* InRandomStream = nullptr ) override;
		inline virtual EParticleDistributionVectorType GetType() const override { return EParticleDistributionVectorType::Uniform; };

		//virtual void GetOutRange(float& MinOut, float& MaxOut) const override
		//{
		//	MinOut = Min;
		//	MaxOut = Max;
		//}

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel) override;
#endif
	};
}