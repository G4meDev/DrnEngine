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

		else if (Type == EParticleDistributionFloatType::Parameter)
		{
			Out = new ParticleDistributionFloatParameter();
		}

		else if (Type == EParticleDistributionFloatType::ConstantCurve)
		{
			Out = new ParticleDistributionFloatConstantCurve();
		}

		drn_check(Out);
		return Out;
	}

#if WITH_EDITOR
	bool ParticleDistributionFloat::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel )
	{
		const char* const Options[] = { "Constant", "Uniform", "Parameter", "Constant Curve" };
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

	float ParticleDistributionFloatConstant::GetValue( float F,ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Constant;
	}

#if WITH_EDITOR
	bool ParticleDistributionFloatConstant::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionFloat::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				bDirty |= ImGui::InputFloat("Value", &Constant);
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
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

	float ParticleDistributionFloatUniform::GetValue( float F,ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Max + (Min - Max) * DIST_GET_RANDOM_VALUE(InRandomStream);
	}

#if WITH_EDITOR
	bool ParticleDistributionFloatUniform::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionFloat::Draw(Ptr, DisplayLabel);
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

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionFloatParameter::Serialize( Archive& Ar )
	{
		ParticleDistributionFloat::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> MinInput;
			Ar >> MaxInput;
			Ar >> MinOutput;
			Ar >> MaxOutput;
			Ar >> Constant;

			Ar >> ParameterName;
			Ar >> *(uint8*)&ParamMode;
		}
		else
		{
			Ar << MinInput;
			Ar << MaxInput;
			Ar << MinOutput;
			Ar << MaxOutput;
			Ar << Constant;

			Ar << ParameterName;
			Ar << (uint8)ParamMode;
		}
	}

	float ParticleDistributionFloatParameter::GetValue( float F, ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		float ParamFloat = 0.f;
		bool bFoundParam = Emitter->Component->GetFloatParameter(ParameterName, ParamFloat);
		if(!bFoundParam)
		{
			ParamFloat = Constant;
		}

		if(ParamMode == EDistributionFloatParamMode::Direct)
		{
			return ParamFloat;
		}
		else if(ParamMode == EDistributionFloatParamMode::Abs)
		{
			ParamFloat = std::abs(ParamFloat);
		}

		float Gradient;
		if(MaxInput <= MinInput)
			Gradient = 0.f;
		else
			Gradient = (MaxOutput - MinOutput)/(MaxInput - MinInput);

		float ClampedParam = std::clamp(ParamFloat, MinInput, MaxInput);
		float Output = MinOutput + ((ClampedParam - MinInput) * Gradient);

		return Output;
	}

#if WITH_EDITOR
	bool ParticleDistributionFloatParameter::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionFloat::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				if (ImGui::InputFloat("Min Input", &MinInput))
				{
					bDirty = true;
					MinInput = std::min(MinInput, MaxInput);
				}

				if (ImGui::InputFloat("Max Input", &MaxInput))
				{
					bDirty = true;
					MaxInput = std::max(MinInput, MaxInput);
				}

				if (ImGui::InputFloat("Min Output", &MinOutput))
				{
					bDirty = true;
					MinOutput = std::min(MinOutput, MaxOutput);
				}

				if (ImGui::InputFloat("Max Output", &MaxOutput))
				{
					bDirty = true;
					MaxOutput = std::max(MinOutput, MaxOutput);
				}

				bDirty |= ImGui::InputFloat( "Constant", &Constant );

				const int32 TextCharLimit = 64;
				char InputText[TextCharLimit];
				strcpy_s(InputText, sizeof(InputText), ParameterName.c_str());

				if ( ImGui::InputText( "Parameter Name", InputText, TextCharLimit ) )
				{
					ParameterName = InputText;
					bDirty = true;
				}

				const char* const Options[] = { "Normal", "Abs", "Direct" };
				int32 Selected = (uint8)ParamMode;
				bDirty |= ImGui::Combo("Parameter Type", &Selected, Options, _countof(Options));
				if (bDirty)
				{
					ParamMode = (EDistributionFloatParamMode)Selected;
				}
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionFloatConstantCurve::Serialize( Archive& Ar )
	{
		ParticleDistributionFloat::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> ConstantCurve;
		}
		else
		{
			Ar << ConstantCurve;
		}
	}

	float ParticleDistributionFloatConstantCurve::GetValue( float F, ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return ConstantCurve.Eval(F, 0.0f);
	}

#if WITH_EDITOR
	bool ParticleDistributionFloatConstantCurve::Draw( TRefCountPtr<ParticleDistributionFloat>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionFloat::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				bDirty |= ConstantCurve.Draw(DisplayLabel);
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

}  // namespace Drn