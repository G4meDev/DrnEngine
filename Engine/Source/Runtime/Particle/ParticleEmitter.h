#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

namespace Drn
{
	class ParticleEmitter : public Serializable, public RefCountedObject
	{
	public:
		ParticleEmitter();

		virtual void Serialize(Archive& Ar) override;

		inline void SetName(const std::string& InName) { Name = InName; }
		inline std::string& GetName() { return Name; }

		inline bool IsEnabled() const { return bEnabled; }
		inline void SetEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

		void RegisterModule(ParticleModule* Module);

#if WITH_EDITOR
		bool Draw();
#endif

		std::vector<ParticleModuleSpawnBase*> SpawningModules;
		std::vector<ParticleModule*> SpawnModules;
		std::vector<ParticleModule*> UpdateModules;

		std::vector<TRefCountPtr<ParticleModule>> Modules;

		int32 ReqInstanceBytes;
		std::unordered_map<ParticleModule*, uint32> ModuleInstanceOffsetMap;

		Vector Origin;
		Quat Rotation;
		bool bUseLocalSpace;
		bool bKillOnDeactivate;
		bool bKillOnCompleted;

		float EmitterDuration;
		float EmitterDurationLow;
		bool bEmitterDurationUseRange;
		int32 EmitterLoops;
		bool bDurationRecalcEachLoop;

		float EmitterDelay;
		float EmitterDelayLow;
		bool bEmitterDelayUseRange;
		bool bDelayFirstLoopOnly;

	private:
		std::string Name;
		bool bEnabled;
	};
}