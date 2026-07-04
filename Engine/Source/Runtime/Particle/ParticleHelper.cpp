#include "DrnPCH.h"
#include "ParticleHelper.h"

#include "Runtime/Particle/ParticleModuleLocation.h"

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


}