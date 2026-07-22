#include "DrnPCH.h"
#include "ParticleEmitter.h"

#include "Runtime/Particle/ParticleModuleEventGenerator.h"
#include "Runtime/Particle/ParticleModuleEventReceiver.h"
#include "Runtime/Particle/ParticleEmitterType.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleEmitter::ParticleEmitter()
		: Name("Emitter")
		, EmitterType(new ParticleEmitterMeshType())
		, bEnabled(true)
		, ReqInstanceBytes(0)
		, Origin(Vector::ZeroVector)
		, Rotation(Quat::Identity)
		, bUseLocalSpace(false)
		, bKillOnDeactivate(false)
		, bKillOnCompleted(false)
		, EmitterDuration(1.0f)
		, EmitterDurationLow(0.0f)
		, bEmitterDurationUseRange(false)
		, EmitterLoops(0)
		, bHasMeshRotation(false)
		, MeshRotationOffset(0)
		, EventGenerator(nullptr)
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
			EmitterType = ParticleEmitterType::Create(Ar);

			int32 ModulesCount;
			Ar >> ModulesCount;
			Modules.resize(ModulesCount);
			for (int32 i = 0; i < ModulesCount; i++)
			{
				EParticleModule ModuleType;
				Ar >> *(uint32*)&ModuleType;

				Modules[i] = (ParticleModuleSpawnBase*)ParticleTypes::CreateParticleModule(ModuleType);
				Modules[i]->Serialize(Ar);
			}

			Ar >> Origin;
			Ar >> Rotation;
			Ar >> bUseLocalSpace;
			Ar >> bKillOnDeactivate;
			Ar >> bKillOnCompleted;

			Ar >> EmitterDuration;
			Ar >> EmitterDurationLow;
			Ar >> bEmitterDurationUseRange;
			Ar >> EmitterLoops;
			Ar >> bDurationRecalcEachLoop;

			Ar >> bHasMeshRotation;

			CalculateRequiredBytesAndOffset();
			for (int32 i = 0; i < ModulesCount; i++)
			{
				RegisterModule(Modules[i]);
			}
		}

		else
		{
			bHasMeshRotation = false;

			const int32 ModulesCount = Modules.size();
			for (int32 i = 0; i < ModulesCount; i++)
			{
				Modules[i]->CompileModule(this);
			}

// ------------------------------------------------------------------------------------

			Ar << Name;
			EmitterType->Serialize(Ar);

			Ar << ModulesCount;
			for (int32 i = 0; i < ModulesCount; i++)
			{
				Ar << (uint32)Modules[i]->GetModuleType();
				Modules[i]->Serialize(Ar);
			}

			Ar << Origin;
			Ar << Rotation;
			Ar << bUseLocalSpace;
			Ar << bKillOnDeactivate;
			Ar << bKillOnCompleted;

			Ar << EmitterDuration;
			Ar << EmitterDurationLow;
			Ar << bEmitterDurationUseRange;
			Ar << EmitterLoops;
			Ar << bDurationRecalcEachLoop;

			Ar << bHasMeshRotation;
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

		if (Module->bEventGenerateModule)
		{
			EventGenerator = static_cast<ParticleModuleEventGenerator*>(Module);
		}

		if (Module->bEventReciverModule)
		{
			EventReceiverModules.push_back(static_cast<ParticleModuleEventReceiverBase*>(Module));
		}

		const int InstanceBytes = Module->RequiredBytesPerInstance();
		if (InstanceBytes > 0)
		{
			ModuleInstanceOffsetMap[Module] = ReqInstanceBytes;
			ReqInstanceBytes += InstanceBytes;
		}

		const int Bytes = Module->RequiredBytes();
		if (Bytes > 0)
		{
			ModuleOffsetMap[Module] = ParticleSize;
			ParticleSize += Bytes;
		}
	}

	void ParticleEmitter::CalculateRequiredBytesAndOffset()
	{
		ParticleSize = sizeof(BaseParticle);

		if (bHasMeshRotation)
		{
			MeshRotationOffset = ParticleSize;
			ParticleSize += sizeof(MeshRotationPayloadData);
		}
	}

#if WITH_EDITOR
	bool ParticleEmitter::Draw()
	{
		bool bDirty = false;

		const int32 EmitterNameCharacterLimit = 64;
		char EmitterName[EmitterNameCharacterLimit];
		strcpy_s(EmitterName, sizeof(EmitterName), GetName().c_str());

		if ( ImGui::InputText( "## ", EmitterName, EmitterNameCharacterLimit) )
		{
			SetName(EmitterName);
			bDirty = true;
		}

		if (ImGui::CollapsingHeader("Type", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= EmitterType->Draw(EmitterType);
		}

		if (ImGui::CollapsingHeader("Emitter", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= Origin.Draw("Origin");
			bDirty |= Rotation.Draw("Rotation");
			bDirty |= ImGui::Checkbox("Use Local Space", &bUseLocalSpace);
			bDirty |= ImGui::Checkbox("Kill On Deactivate", &bKillOnDeactivate);
			bDirty |= ImGui::Checkbox("Kill On Completed", &bKillOnCompleted);
		}

		if (ImGui::CollapsingHeader("Duration", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= ImGui::InputFloat("Emitter Duration", &EmitterDuration);
			bDirty |= ImGui::InputFloat("Emitter Duration Low", &EmitterDurationLow);
			bDirty |= ImGui::Checkbox("Emitter Duration Use Range", &bEmitterDurationUseRange);
			bDirty |= ImGui::Checkbox("Duration Recalculate Each Loop", &bDurationRecalcEachLoop);
			bDirty |= ImGui::InputInt("Emitter Loops", &EmitterLoops);
		}

		return bDirty;
	}
#endif

        }  // namespace Drn