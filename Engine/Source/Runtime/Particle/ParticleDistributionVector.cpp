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

		else if (Type == EParticleDistributionVectorType::Parameter)
		{
			Out = new ParticleDistributionVectorParameter();
		}

		else if (Type == EParticleDistributionVectorType::ConstantCurve)
		{
			Out = new ParticleDistributionVectorConstantCurve();
		}

		drn_check(Out);
		return Out;
	}

#if WITH_EDITOR
	bool ParticleDistributionVector::Draw( TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel )
	{
		const char* const Options[] = { "Constant", "Uniform", "Parameter", "Constant Curve" };
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

	Vector ParticleDistributionVectorConstant::GetValue( float F, ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
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

	Vector ParticleDistributionVectorUniform::GetValue( float F, ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return Max + (Min - Max) * Vector(DIST_GET_RANDOM_VALUE(InRandomStream), DIST_GET_RANDOM_VALUE(InRandomStream), DIST_GET_RANDOM_VALUE(InRandomStream));
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

	void ParticleDistributionVectorParameter::Serialize( Archive& Ar )
	{
		ParticleDistributionVector::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> MinInput;
			Ar >> MaxInput;
			Ar >> MinOutput;
			Ar >> MaxOutput;
			Ar >> Constant;

			Ar >> ParameterName;
			Ar >> *(uint8*)&ParamModes[0];
			Ar >> *(uint8*)&ParamModes[1];
			Ar >> *(uint8*)&ParamModes[2];
		}
		else
		{
			Ar << MinInput;
			Ar << MaxInput;
			Ar << MinOutput;
			Ar << MaxOutput;
			Ar << Constant;

			Ar << ParameterName;
			Ar << *(uint8*)&ParamModes[0];
			Ar << *(uint8*)&ParamModes[1];
			Ar << *(uint8*)&ParamModes[2];
		}
	}

	Vector ParticleDistributionVectorParameter::GetValue( float F, ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		Vector ParamVector(0.f);
		bool bFoundParam = Emitter->Component->GetVectorParameter(ParameterName, ParamVector);
		if(!bFoundParam)
		{
			ParamVector = Constant;
		}

		if(ParamModes[0] == EDistributionVectorParamMode::Abs)
		{
			ParamVector.SetX(std::abs(ParamVector.GetX()));
		}

		if(ParamModes[1] == EDistributionVectorParamMode::Abs)
		{
			ParamVector.SetY(std::abs(ParamVector.GetY()));
		}

		if(ParamModes[2] == EDistributionVectorParamMode::Abs)
		{
			ParamVector.SetZ(std::abs(ParamVector.GetZ()));
		}

		Vector Gradient;
		if(MaxInput.GetX() <= MinInput.GetX())
			Gradient.SetX(0.f);
		else
			Gradient.SetX((MaxOutput.GetX() - MinOutput.GetX())/(MaxInput.GetX() - MinInput.GetX()));

		if(MaxInput.GetY() <= MinInput.GetY())
			Gradient.SetY(0.f);
		else
			Gradient.SetY((MaxOutput.GetY() - MinOutput.GetY())/(MaxInput.GetY() - MinInput.GetY()));

		if(MaxInput.GetZ() <= MinInput.GetZ())
			Gradient.SetZ(0.f);
		else
			Gradient.SetZ((MaxOutput.GetZ() - MinOutput.GetZ())/(MaxInput.GetZ() - MinInput.GetZ()));

		Vector ClampedParam;
		ClampedParam.SetX(std::clamp(ParamVector.GetX(), MinInput.GetX(), MaxInput.GetX()));
		ClampedParam.SetY(std::clamp(ParamVector.GetY(), MinInput.GetY(), MaxInput.GetY()));
		ClampedParam.SetZ(std::clamp(ParamVector.GetZ(), MinInput.GetZ(), MaxInput.GetZ()));

		Vector Output = MinOutput + ((ClampedParam - MinInput) * Gradient);

		if(ParamModes[0] == EDistributionVectorParamMode::Direct)
		{
			Output.SetX(ParamVector.GetX());
		}

		if(ParamModes[1] == EDistributionVectorParamMode::Direct)
		{
			Output.SetY(ParamVector.GetY());
		}

		if(ParamModes[2] == EDistributionVectorParamMode::Direct)
		{
			Output.SetZ(ParamVector.GetZ());
		}

		return Output;
	}

#if WITH_EDITOR
	bool ParticleDistributionVectorParameter::Draw( TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionVector::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				if (MinInput.Draw(DisplayLabel, "Min Input", EParameterPopupContext::None))
				{
					bDirty = true;
					MinInput = MinInput.ComponentMin(MaxInput);
				}

				if (MaxInput.Draw(DisplayLabel, "Max Input", EParameterPopupContext::None))
				{
					bDirty = true;
					MaxInput = MaxInput.ComponentMax(MinInput);
				}

				if (MinOutput.Draw(DisplayLabel, "Min Output", EParameterPopupContext::None))
				{
					bDirty = true;
					MinOutput = MinOutput.ComponentMin(MaxOutput);
				}

				if (MaxOutput.Draw(DisplayLabel, "Max Output", EParameterPopupContext::None))
				{
					bDirty = true;
					MaxOutput = MaxOutput.ComponentMax(MinOutput);
				}

				bDirty |= Constant.Draw(DisplayLabel, "Constant", EParameterPopupContext::None);

				const int32 TextCharLimit = 64;
				char InputText[TextCharLimit];
				strcpy_s(InputText, sizeof(InputText), ParameterName.c_str());

				if ( ImGui::InputText( "Parameter Name", InputText, TextCharLimit ) )
				{
					ParameterName = InputText;
					bDirty = true;
				}

				const char* const Options[] = { "Normal", "Abs", "Direct" };
				const char* const Axes[] = { "X", "Y", "Z" };

				for (int32 i = 0; i < 3; i++)
				{
					int32 Selected = (uint8)ParamModes[i];
					bDirty |= ImGui::Combo(std::format("Parameter Type {}", Axes[i]).c_str(), &Selected, Options, _countof(Options));
					if (bDirty)
					{
						ParamModes[i] = (EDistributionVectorParamMode)Selected;
					}
				}
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

// ---------------------------------------------------------------------------------------------

	void ParticleDistributionVectorConstantCurve::Serialize( Archive& Ar )
	{
		ParticleDistributionVector::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> ConstantCurve;
		}
		else
		{
			Ar << ConstantCurve;
		}
	}

	Vector ParticleDistributionVectorConstantCurve::GetValue( float F, ParticleEmitterInstance* Emitter, RandomStream* InRandomStream )
	{
		return ConstantCurve.Eval(F, Vector::ZeroVector);
	}

#if WITH_EDITOR
	bool ParticleDistributionVectorConstantCurve::Draw( TRefCountPtr<ParticleDistributionVector>& Ptr, const std::string& DisplayLabel )
	{
		if (ImGui::CollapsingHeader(DisplayLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushID(DisplayLabel.c_str());

			bool bDirty = ParticleDistributionVector::Draw(Ptr, DisplayLabel);
			if (!bDirty)
			{
				bDirty = ConstantCurve.Draw(DisplayLabel);
			}

			ImGui::PopID();

			return bDirty;
		}

		return false;
	}
#endif

}  // namespace Drn