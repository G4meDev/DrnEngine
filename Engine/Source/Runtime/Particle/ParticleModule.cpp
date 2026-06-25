#include "DrnPCH.h"
#include "ParticleModule.h"

namespace Drn
{
	bool ParticleModuleSpawn::GetSpawnAmount( ParticleEmitterInstance* Owner, float OldLeftover, float DeltaTime, float& Rate )
	{
		drn_check(Owner);

		Rate = SpawnRate;
		return true;
	}

}