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

#if WITH_EDITOR
	bool ParticleModuleSpawn::Draw( ParticleEmitterInstance* Owner )
	{
		ImGui::Text("Test");

		return false;
	}
#endif
        }