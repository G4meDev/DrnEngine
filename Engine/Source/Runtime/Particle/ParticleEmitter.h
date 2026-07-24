#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

namespace Drn
{
	class ParticleEmitterType;

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

		void CalculateRequiredBytesAndOffset();

		inline bool IsMeshRotationActive() const { return bHasMeshRotation; }
		inline int32 GetMeshRotationOffset() const { return MeshRotationOffset; }

		inline ParticleEmitterType* GetEmitterType() const { return EmitterType; }

		bool IsMeshEmitter() const;
		bool IsCpuSpriteEmitter() const;

#if WITH_EDITOR
		bool Draw();
#endif

		TRefCountPtr<ParticleEmitterType> EmitterType;

		std::vector<ParticleModuleSpawnBase*> SpawningModules;
		std::vector<ParticleModule*> SpawnModules;
		std::vector<ParticleModule*> UpdateModules;

		class ParticleModuleEventGenerator* EventGenerator;
		std::vector<class ParticleModuleEventReceiverBase*> EventReceiverModules;

		std::vector<TRefCountPtr<ParticleModule>> Modules;

		int32 ReqInstanceBytes;
		std::unordered_map<ParticleModule*, uint32> ModuleOffsetMap;
		std::unordered_map<ParticleModule*, uint32> ModuleInstanceOffsetMap;

		Vector Origin;
		Quat Rotation;
		bool bUseLocalSpace;
		bool bKillOnDeactivate;
		bool bKillOnCompleted;
		EParticleSortMode SortMode;
		int32 MaxParticleCount;

		float EmitterDuration;
		float EmitterDurationLow;
		bool bEmitterDurationUseRange;
		int32 EmitterLoops;
		bool bDurationRecalcEachLoop;

		bool bHasMeshRotation;

		int32 ParticleSize;
		int32 MeshRotationOffset;

	private:
		std::string Name;
		bool bEnabled;
	};
}