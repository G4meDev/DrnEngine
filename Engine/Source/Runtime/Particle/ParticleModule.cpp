#include "DrnPCH.h"
#include "ParticleModule.h"

namespace Drn
{
	void ParticleModule::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> bEnabled;
		}

		else
		{
			Ar << bEnabled;
		}
	}

RandomStream& ParticleModule::GetRandomStream( ParticleEmitterInstance* Owner )
	{
		return Owner->EmitterRandomStream;
	}

}