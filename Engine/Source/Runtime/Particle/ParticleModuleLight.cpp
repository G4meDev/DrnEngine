#include "DrnPCH.h"
#include "ParticleModuleLight.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleLight::ParticleModuleLight()
		: ParticleModule()
		, ColorScaleOverLife(new ParticleDistributionVectorConstant(Vector::OneVector))
		, BrightnessOverLife(new ParticleDistributionFloatConstant(1.0f))
		, RadiusScale(new ParticleDistributionFloatConstant(1.0f))
		, bUseAbsoluteAttributes(false)
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleLight::CompileModule( ParticleEmitter* Emitter )
	{
		Emitter->bHasLight = true;
	}

	void ParticleModuleLight::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		const int32 LightOffset = Owner->Emitter->GetLightOffset();
		if (LightOffset)
		{
			SPAWN_INIT;
			{
				ParticleLightPayload* PayloadData = (ParticleLightPayload*)((uint8*)&Particle + LightOffset);

				Vector Color	= ColorScaleOverLife->GetValue(ParticleBase->RelativeTime, Owner);
				float Brighness	= BrightnessOverLife->GetValue(ParticleBase->RelativeTime, Owner);
				float Radius	= RadiusScale->GetValue(ParticleBase->RelativeTime, Owner);

				if (bUseAbsoluteAttributes)
				{
					PayloadData->Color = Color * Brighness;
					PayloadData->Radius = Radius;
				}
				else
				{
					PayloadData->Color = Color * Vector(ParticleBase->Color.GetX(), ParticleBase->Color.GetY(), ParticleBase->Color.GetZ()) * Brighness * ParticleBase->Color.GetW();
					PayloadData->Radius = Radius * ParticleBase->Size.GetMaxComponent();
				}
			}
		}
	}

	void ParticleModuleLight::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		const int32 LightOffset = Owner->Emitter->GetLightOffset();
		if (LightOffset)
		{
			if (bUseAbsoluteAttributes)
			{
				BEGIN_UPDATE_LOOP;
				{
					ParticleLightPayload* PayloadData = (ParticleLightPayload*)((uint8*)&Particle + LightOffset);

					Vector Color	= ColorScaleOverLife->GetValue(Particle.RelativeTime, Owner);
					float Brighness	= BrightnessOverLife->GetValue(Particle.RelativeTime, Owner);
					float Radius	= RadiusScale->GetValue(Particle.RelativeTime, Owner);

					PayloadData->Color = Color * Brighness;
					PayloadData->Radius = Radius;
				}
				END_UPDATE_LOOP;
			}
			else
			{
				BEGIN_UPDATE_LOOP;
				{
					ParticleLightPayload* PayloadData = (ParticleLightPayload*)((uint8*)&Particle + LightOffset);

					Vector Color	= ColorScaleOverLife->GetValue(Particle.RelativeTime, Owner);
					float Brighness	= BrightnessOverLife->GetValue(Particle.RelativeTime, Owner);
					float Radius	= RadiusScale->GetValue(Particle.RelativeTime, Owner);

					PayloadData->Color = Color * Vector(Particle.Color.GetX(), Particle.Color.GetY(), Particle.Color.GetZ()) * Brighness * Particle.Color.GetW();
					PayloadData->Radius = Radius * Particle.Size.GetMaxComponent();
				}
				END_UPDATE_LOOP;
			}
		}
	}

	void ParticleModuleLight::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			ColorScaleOverLife = ParticleDistributionVector::Create(Ar);
			BrightnessOverLife = ParticleDistributionFloat::Create(Ar);
			RadiusScale = ParticleDistributionFloat::Create(Ar);
			Ar >> bUseAbsoluteAttributes;
		}
		else
		{
			ColorScaleOverLife->Serialize(Ar);
			BrightnessOverLife->Serialize(Ar);
			RadiusScale->Serialize(Ar);
			Ar << bUseAbsoluteAttributes;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleLight::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= ImGui::Checkbox("Use Absolute Attributes", &bUseAbsoluteAttributes);
		bDirty |= ColorScaleOverLife->Draw(ColorScaleOverLife, "Color Scale Over Life");
		bDirty |= BrightnessOverLife->Draw(BrightnessOverLife, "Brightness Over Life");
		bDirty |= RadiusScale->Draw(RadiusScale, "Radius Scale");

		return bDirty;
	}
#endif

}  // namespace Drn