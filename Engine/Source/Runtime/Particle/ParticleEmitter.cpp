#include "DrnPCH.h"
#include "ParticleEmitter.h"

namespace Drn
{
	ParticleEmitter::ParticleEmitter()
		: Name("Emitter")
		, bEnabled(true)
		, ReqInstanceBytes(0)
	{
		
	}

	void ParticleEmitter::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			ReqInstanceBytes = 0;
			SpawningModules.clear();
			SpawnModules.clear();
			UpdateModules.clear();

			Ar >> Name;

			int32 ModulesCount;
			Ar >> ModulesCount;
			Modules.resize(ModulesCount);
			for (int32 i = 0; i < ModulesCount; i++)
			{
				EParticleModule ModuleType;
				Ar >> *(uint32*)&ModuleType;

				Modules[i] = (ParticleModuleSpawnBase*)ParticleTypes::CreateParticleModule(ModuleType);
				Modules[i]->Serialize(Ar);
				RegisterModule(Modules[i]);
			}
		}

		else
		{
			Ar << Name;

			const int32 ModulesCount = Modules.size();
			Ar << ModulesCount;
			for (int32 i = 0; i < ModulesCount; i++)
			{
				Ar << (uint32)Modules[i]->GetModuleType();
				Modules[i]->Serialize(Ar);
			}
		}
	}

	void ParticleEmitter::RegisterModule( ParticleModule* Module )
	{
		drn_check(Module);

		if (!Module->IsEffectiveModule())
		{
			return;
		}

		if (Module->bSpawningModule)
		{
			SpawningModules.push_back(static_cast<ParticleModuleSpawnBase*>(Module));
		}

		if (Module->bSpawnModule)
		{
			SpawnModules.push_back(Module);
		}

		if (Module->bUpdateModule)
		{
			UpdateModules.push_back(Module);
		}

		const int InstanceBytes = Module->RequiredBytesPerInstance();
		if (InstanceBytes > 0)
		{
			ModuleInstanceOffsetMap[Module] = ReqInstanceBytes;
			ReqInstanceBytes += InstanceBytes;
		}
	}

        }