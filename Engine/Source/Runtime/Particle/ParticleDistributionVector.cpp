#include "DrnPCH.h"
#include "ParticleDistributionVector.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	#define DIST_GET_RANDOM_VALUE(RandStream)		((RandStream == NULL) ? Math::SRand() : RandStream->GetFraction())
	
	void ParticleDistributionVector::Serialize( Archive& Ar )
	{
		if (!Ar.IsLoading())
		{
			Ar << (uint8)GetType();
		}
	}

	ParticleDistributionVector* ParticleDistributionVector::Create( Archive& Ar )
	{
		drn_check(Ar.IsLoading());

		EParticleDistributionVectorType Type;
		Ar >> *(uint8*)&Type;

		ParticleDistributionVector* Out = Create(Type);

		if (Out)
		{
			Out->Serialize(Ar);
		}

		return Out;
	}

	ParticleDistributionVector* ParticleDistributionVector::Create( EParticleDistributionVectorType Type )
	{
		ParticleDistributionVector* Out = nullptr;

		if (Type == EParticleDistributionVectorType::Constant)
		{
			Out = new ParticleDistributionVectorConstant();
		}

		else if (Type == EParticleDistributionVectorType::Uniform)
		{
			Out = new ParticleDistributionVectorUniform();
		}

		drn_check(Out);
		return Out;
	}

#if WITH_EDITOR
	bool ParticleDistributionVector::Draw( TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel )
	{
		const char* const Options[] = { "Constant", "Uniform" };
		int32 Selected = (uint8)GetType();
		bool bDirty = ImGui::Combo("Distribution Type", &Selected, Options, _countof(Options));
		if (bDirty)
		{
			Ptr = Create((EParticleDistributionVectorType)Selected);
		}

		return bDirty;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionVectorConstant::Serialize( Archive& Ar )
	{
		ParticleDistributionVector::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Constant;
		}
		else
		{
			Ar << Constant;
		}
	}

	Vector ParticleDistributionVectorConstant::GetValue( ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Constant;
	}

#if WITH_EDITOR
	bool ParticleDistributionVectorConstant::Draw( TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionVector::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				bDirty |= Constant.Draw(DisplayLabel, "Value", EParameterPopupContext::None);
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionVectorUniform::Serialize( Archive& Ar )
	{
		ParticleDistributionVector::Serialize(Ar);

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

	Vector ParticleDistributionVectorUniform::GetValue( ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Max + (Min - Max) * DIST_GET_RANDOM_VALUE(InRandomStream);
	}

#if WITH_EDITOR
	bool ParticleDistributionVectorUniform::Draw( TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionVector::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				if (Min.Draw(DisplayLabel, "Min", EParameterPopupContext::None))
				{
					bDirty = true;
					Min = Min.ComponentMin(Max);
				}

				if (Max.Draw(DisplayLabel, "Max", EParameterPopupContext::None))
				{
					bDirty = true;
					Max = Max.ComponentMax(Min);
				}
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

// ---------------------------------------------------------------------------------------------

}  // namespace Drn