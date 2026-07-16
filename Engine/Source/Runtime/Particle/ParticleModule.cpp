#include "DrnPCH.h"
#include "ParticleModule.h"

namespace Drn
{
	void ParticleModule::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> bEnabled;
			Ar >> bValid;
		}

		else
		{
			Ar << bEnabled;
			Ar << bValid;
		}
	}

RandomStream& ParticleModule::GetRandomStream( ParticleEmitterInstance* Owner )
	{
		return Owner->EmitterRandomStream;
	}

}