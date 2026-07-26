#include "DrnPCH.h"
#include "ParticleModuleKill.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleKillHeight::ParticleModuleKillHeight()
		: ParticleModule()
		, Height(new ParticleDistributionFloatConstant(-10.0f))
		, bAbsolute(false)
		, bFloor(true)
		, bApplyPSysScale(false)
	{
		bUpdateModule = true;
	}

	void ParticleModuleKillHeight::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Height = ParticleDistributionFloat::Create(Ar);
			Ar >> bAbsolute;
			Ar >> bFloor;
			Ar >> bApplyPSysScale;
		}
		else
		{
			Height->Serialize(Ar);
			Ar << bAbsolute;
			Ar << bFloor;
			Ar << bApplyPSysScale;
		}
	}

	void ParticleModuleKillHeight::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		float CheckHeight = Height->GetValue(Owner->EmitterTime, Owner);
		if (bApplyPSysScale)
		{
			Vector OwnerScale = Owner->Component->GetWorldScale();
			CheckHeight *= OwnerScale.Y;
		}

		if (!bAbsolute)
		{
			CheckHeight += Owner->Component->GetWorldLocation().Y;
		}

		BEGIN_UPDATE_LOOP;
		{
			Vector Position = Particle.Location;

			if (Owner->Emitter->bUseLocalSpace)
			{
				Position = Owner->Component->GetWorldTransform().TransformVector(Position);
			}

			if (bFloor && (Position.Y < CheckHeight))
			{
				Owner->KillParticle(i);
			}
			else if (!bFloor && (Position.Y > CheckHeight))
			{
				Owner->KillParticle(i);
			}
		}
		END_UPDATE_LOOP;
	}

#if WITH_EDITOR
	bool ParticleModuleKillHeight::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		bDirty |= ImGui::Checkbox("Absolute", &bAbsolute);
		bDirty |= ImGui::Checkbox("Floor", &bFloor);
		bDirty |= ImGui::Checkbox("Apply System Scale", &bApplyPSysScale);
		bDirty |= Height->Draw(Height, "Height");

		return bDirty;
	}
#endif

}  // namespace Drn