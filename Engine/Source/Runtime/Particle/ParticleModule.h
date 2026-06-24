#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitter;
	class ParticleEmitterInstance;

	class ParticleModule : public RefCountedObject
	{
	public:
		uint8 bSpawnModule:1;
		uint8 bUpdateModule:1;
		uint8 bFinalUpdateModule:1;
		uint8 bUpdateForGPUEmitter:1;

		uint8 bEnabled:1;
		uint8 bEditable:1;

		int32 RandomSeed;

		virtual void CompileModule(ParticleEmitter* Emitter) = 0;
		virtual void Spawn(ParticleEmitterInstance* EmitterInstance, float SpawnTime) = 0;
		virtual void Update(ParticleEmitterInstance* EmitterInstance, float DeltaTime) = 0;
		virtual void FinalUpdate(ParticleEmitterInstance* EmitterInstance, float DeltaTime) = 0;

	};
}