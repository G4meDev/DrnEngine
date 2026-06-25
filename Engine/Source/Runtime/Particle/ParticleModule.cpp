#include "DrnPCH.h"
#include "ParticleModule.h"

namespace Drn
{
	RandomStream& ParticleModule::GetRandomStream( ParticleEmitterInstance* Owner )
	{
		return Owner->EmitterRandomStream;
	}

}