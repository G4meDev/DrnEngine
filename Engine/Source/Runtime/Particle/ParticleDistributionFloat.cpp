#include "DrnPCH.h"
#include "ParticleDistributionFloat.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	#define DIST_GET_RANDOM_VALUE(RandStream)		((RandStream == NULL) ? Math::SRand() : RandStream->GetFraction())
	
	void ParticleDistributionFloat::Serialize( Archive& Ar )
	{
		if (!Ar.IsLoading())
		{
			Ar << (uint8)GetType();
		}
	}

	ParticleDistributionFloat* ParticleDistributionFloat::Create( Archive& Ar )
	{
		drn_check(Ar.IsLoading());

		EParticleDistributionFloatType Type;
		Ar >> *(uint8*)&Type;

		ParticleDistributionFloat* Out = Create(Type);

		if (Out)
		{
			Out->Serialize(Ar);
		}

		return Out;
	}

	ParticleDistributionFloat* ParticleDistributionFloat::Create( EParticleDistributionFloatType Type )
	{
		ParticleDistributionFloat* Out = nullptr;

		if (Type == EParticleDistributionFloatType::Constant)
		{
			Out = new ParticleDistributionFloatConstant();
		}

		else if (Type == EParticleDistributionFloatType::Uniform)
		{
			Out = new ParticleDistributionFloatUniform();
		}

		drn_check(Out);
		return Out;
	}

#if WITH_EDITOR
	bool ParticleDistributionFloat::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr )
	{
		const char* const Options[] = { "Constant", "Uniform" };
		int32 Selected = (uint8)GetType();
		bool bDirty = ImGui::Combo("Distribution Type", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			Ptr = Create((EParticleDistributionFloatType)Selected);
		}

		return bDirty;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionFloatConstant::Serialize( Archive& Ar )
	{
		ParticleDistributionFloat::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Constant;
		}
		else
		{
			Ar << Constant;
		}
	}

	float ParticleDistributionFloatConstant::GetValue( ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Constant;
	}

#if WITH_EDITOR
	bool ParticleDistributionFloatConstant::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr )
	{
		bool bDirty = ParticleDistributionFloat::Draw(Ptr);
		if (!bDirty)
		{
			bDirty |= ImGui::InputFloat("Value", &Constant);
		}

		return bDirty;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionFloatUniform::Serialize( Archive& Ar )
	{
		ParticleDistributionFloat::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Min;
			Ar >> Max;
		}
		else
		{
			Ar << Min;
			Ar << Max;
		}
	}

	float ParticleDistributionFloatUniform::GetValue( ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Max + (Min - Max) * DIST_GET_RANDOM_VALUE(InRandomStream);
	}

#if WITH_EDITOR
	bool ParticleDistributionFloatUniform::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr )
	{
		bool bDirty = ParticleDistributionFloat::Draw(Ptr);
		if (!bDirty)
		{
			if (ImGui::InputFloat("Min", &Min))
			{
				bDirty = true;
				Min = std::min(Min, Max);
			}

			if (ImGui::InputFloat("Max", &Max))
			{
				bDirty = true;
				Max = std::max(Min, Max);
			}
		}

		return bDirty;
	}
#endif

// ---------------------------------------------------------------------------------------------

}  // namespace Drn