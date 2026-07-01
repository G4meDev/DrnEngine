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

			int32 SpawningModuleCount;
			Ar >> SpawningModuleCount;
			SpawningModules.resize(SpawningModuleCount);
			for (int32 i = 0; i < SpawningModuleCount; i++)
			{
				EParticleModule ModuleType;
				Ar >> *(uint32*)&ModuleType;

				SpawningModules[i] = (ParticleModuleSpawnBase*)ParticleTypes::CreateParticleModule(ModuleType);
				SpawningModules[i]->Serialize(Ar);
			}

			int32 SpawnModuleCount;
			Ar >> SpawnModuleCount;
			SpawnModules.resize(SpawnModuleCount);
			for (int32 i = 0; i < SpawnModuleCount; i++)
			{
				EParticleModule ModuleType;
				Ar >> *(uint32*)&ModuleType;

				SpawnModules[i] = ParticleTypes::CreateParticleModule(ModuleType);
				SpawnModules[i]->Serialize(Ar);
			}

			int32 UpdateModuleCount;
			Ar >> UpdateModuleCount;
			UpdateModules.resize(UpdateModuleCount);
			for (int32 i = 0; i < UpdateModuleCount; i++)
			{
				EParticleModule ModuleType;
				Ar >> *(uint32*)&ModuleType;

				UpdateModules[i] = ParticleTypes::CreateParticleModule(ModuleType);
				UpdateModules[i]->Serialize(Ar);
			}
		}

		else
		{
			Ar << Name;

			const int32 SpawningModuleCount = SpawningModules.size();
			Ar << SpawningModuleCount;
			for (int32 i = 0; i < SpawningModuleCount; i++)
			{
				Ar << (uint32)SpawningModules[i]->GetModuleType();
				SpawningModules[i]->Serialize(Ar);
			}

			const int32 SpawnModuleCount = SpawnModules.size();
			Ar << SpawnModuleCount;
			for (int32 i = 0; i < SpawnModuleCount; i++)
			{
				Ar << (uint32)SpawnModules[i]->GetModuleType();
				SpawnModules[i]->Serialize(Ar);
			}

			const int32 UpdateModuleCount = UpdateModules.size();
			Ar << UpdateModuleCount;
			for (int32 i = 0; i < UpdateModuleCount; i++)
			{
				Ar << (uint32)UpdateModules[i]->GetModuleType();
				UpdateModules[i]->Serialize(Ar);
			}
		}
	}

}