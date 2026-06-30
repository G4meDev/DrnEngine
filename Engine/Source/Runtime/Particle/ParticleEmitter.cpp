#include "DrnPCH.h"
#include "ParticleEmitter.h"

namespace Drn
{
	ParticleEmitter::ParticleEmitter()
		: Name("Emitter")
		, bEnabled(true)
	{
		
	}

	void ParticleEmitter::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> Name;
		}

		else
		{
			Ar << Name;
		}
	}

}