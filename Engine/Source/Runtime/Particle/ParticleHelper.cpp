#include "DrnPCH.h"
#include "ParticleHelper.h"

#include "Runtime/Particle/ParticleModuleLocation.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	std::function<ParticleModule*()> ParticleTypes::ParticleModuleFactory[(int32)EParticleModule::Max];

#if WITH_EDITOR
	ParticleModuleMetaData              ParticleTypes::ParticleModulesMetaData[(int32)EParticleModule::Max];
	std::vector<ParticleModuleCategory> ParticleTypes::ParticleModuleCategories;
#endif

	template<typename T>
	void ParticleTypes::RegisterParticleModule(EParticleModule Module, const std::string& DisplayName, const std::string& CategoryName)
	{
		const int32 ModuleIndex = (int32)Module;
		ParticleModuleFactory[ModuleIndex] = []() { return new T(); };

#if WITH_EDITOR
		ParticleModulesMetaData[ModuleIndex] = ParticleModuleMetaData(DisplayName);

		ParticleModuleCategory* Category = nullptr;
		for (ParticleModuleCategory& Cat : ParticleModuleCategories)
		{
			if (Cat.CategoryName == CategoryName)
			{
				Category = &Cat;
			}
		}

		if (!Category)
		{
			ParticleModuleCategories.push_back(CategoryName);
			Category = &ParticleModuleCategories.back();
		}

		Category->Modules.push_back(Module);
#endif
	}

	void ParticleTypes::RegisterParticleModules()
	{
		RegisterParticleModule<ParticleModuleSpawn>(EParticleModule::Spawn, "Spawn", "Spawn");
		RegisterParticleModule<ParticleModuleSpawnPerUnit>(EParticleModule::SpawnPerUnit, "Spawn Per Unit", "Spawn");
		RegisterParticleModule<ParticleModuleLocationPrimitiveSphere>(EParticleModule::LocationSphere, "Sphere", "Location");


	}

	Archive& operator<<(Archive& Ar, ParticleBurst& Data)
	{
		Ar << Data.Count;
		Ar << Data.CountLow;
		Ar << Data.Time;

		return Ar;
	}

	Archive& operator>>(Archive& Ar, ParticleBurst& Data)
	{
		Ar >> Data.Count;
		Ar >> Data.CountLow;
		Ar >> Data.Time;

		return Ar;
	}

#if WITH_EDITOR
	bool ParticleBurst::Draw()
	{
		bool bDirty = false;

		bDirty |= ImGui::InputInt("Count", &Count);
		bDirty |= ImGui::InputInt("Count Low", &CountLow);
		bDirty |= ImGui::InputFloat("Time", &Time);

		return bDirty;
	}
#endif

        }