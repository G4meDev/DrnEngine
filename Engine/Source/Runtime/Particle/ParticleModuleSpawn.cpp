#include "DrnPCH.h"
#include "ParticleModuleSpawn.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	bool ParticleModuleSpawn::GetSpawnAmount( ParticleEmitterInstance* Owner, float OldLeftover, float DeltaTime, float& Rate )
	{
		drn_check(Owner);

		Rate = SpawnRate;
		return true;
	}

	void ParticleModuleSpawn::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> SpawnRate;
		}

		else
		{
			Ar << SpawnRate;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleSpawn::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleSpawnBase::Draw(Owner);

		bDirty |= ImGui::InputFloat("Spawn Rate", &SpawnRate);

		return bDirty;
	}
#endif
        }