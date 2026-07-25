#include "DrnPCH.h"
#include "ParticleHelper.h"

#include "Runtime/Particle/ParticleModuleLocation.h"
#include "Runtime/Particle/ParticleModuleLifetime.h"
#include "Runtime/Particle/ParticleModuleVelocity.h"
#include "Runtime/Particle/ParticleModuleAcceleration.h"
#include "Runtime/Particle/ParticleModuleMeshRotation.h"
#include "Runtime/Particle/ParticleModuleEventGenerator.h"
#include "Runtime/Particle/ParticleModuleEventReceiver.h"
#include "Runtime/Particle/ParticleModuleCollision.h"
#include "Runtime/Particle/ParticleModuleSize.h"
#include "Runtime/Particle/ParticleModuleColor.h"
#include "Runtime/Particle/ParticleModuleSubuv.h"

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

		RegisterParticleModule<ParticleModuleLocation>(EParticleModule::Location, "Initial Location", "Location");
		RegisterParticleModule<ParticleModuleLocationPrimitiveSphere>(EParticleModule::LocationSphere, "Sphere", "Location");

		RegisterParticleModule<ParticleModuleLifetime>(EParticleModule::Lifetime, "Lifetime", "Lifetime");

		RegisterParticleModule<ParticleModuleVelocity>(EParticleModule::Velocity, "Initial Velocity", "Velocity");
		RegisterParticleModule<ParticleModuleVelocityOverLifetime>(EParticleModule::VelocityOverLifetime, "Velocity Over Lifetime", "Velocity");

		RegisterParticleModule<ParticleModuleAccelerationConstant>(EParticleModule::AccelerationConstant, "Acceleration Constant", "Acceleration");
		RegisterParticleModule<ParticleModuleAcceleration>(EParticleModule::Acceleration, "Acceleration", "Acceleration");
		RegisterParticleModule<ParticleModuleAccelerationOverLife>(EParticleModule::AccelerationOverLife, "Acceleration Over Life", "Acceleration");
		RegisterParticleModule<ParticleModuleDrag>(EParticleModule::Drag, "Drag", "Acceleration");

		RegisterParticleModule<ParticleModuleMeshRotation>(EParticleModule::MeshRotation, "Mesh Rotation", "Mesh Rotation");
		RegisterParticleModule<ParticleModuleMeshRotationRate>(EParticleModule::MeshRotationRate, "Mesh Rotation Rate", "Mesh Rotation");
		RegisterParticleModule<ParticleModuleMeshRotationRateOverLifetime>(EParticleModule::MeshRotationRateOverLifetime, "Mesh Rotation Rate Over Lifetime", "Mesh Rotation");
		RegisterParticleModule<ParticleModuleMeshRotationRateMultiplyLifetime>(EParticleModule::MeshRotationRateMultiplyLifetime, "Mesh Rotation Rate Multiply Lifetime", "Mesh Rotation");

		RegisterParticleModule<ParticleModuleEventGenerator>(EParticleModule::EventGenerator, "Event Generator", "Event");
		RegisterParticleModule<ParticleModuleEventReceiverKillParticles>(EParticleModule::EventReceiverKillParticles, "Event Receiver Kill Particles", "Event");
		RegisterParticleModule<ParticleModuleEventReceiverSpawn>(EParticleModule::EventReceiverSpawn, "Event Receiver Spawn", "Event");

		RegisterParticleModule<ParticleModuleCollision>(EParticleModule::Collision, "Collision", "Collision");

		RegisterParticleModule<ParticleModuleSize>(EParticleModule::Size, "Initial Size", "Size");
		RegisterParticleModule<ParticleModuleSizeScale>(EParticleModule::SizeScale, "Size Scale", "Size");
		RegisterParticleModule<ParticleModuleSizeByLife>(EParticleModule::SizeByLife, "Size By Life", "Size");

		RegisterParticleModule<ParticleModuleColor>(EParticleModule::Color, "Color", "Color");
		RegisterParticleModule<ParticleModuleColorOverLife>(EParticleModule::ColorOverLife, "Color Over Life", "Color");

		RegisterParticleModule<ParticleModuleSubuv>(EParticleModule::Subuv, "SubImage Index", "Subuv");


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