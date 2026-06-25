#pragma once

#include "ForwardTypes.h"
#include "Runtime/Particle/ParticleHelper.h"
#include "Runtime/Particle/ParticleModule.h"
#include "Runtime/Particle/ParticleModuleSpawn.h"

namespace Drn
{
	class ParticleSystemComponent;
	class ParticleEmitter;
	//class ParticleModule;
	class BaseParticle;

	class ParticleEmitterInstance : public RefCountedObject
	{
	public:
		ParticleEmitterInstance();
		virtual ~ParticleEmitterInstance();

		ParticleEmitter* Emitter;
		ParticleSystemComponent* Component;
		EEmitterType EmitterType;
		Vector Location;
		Vector OldLocation;
		Matrix EmitterToSimulation;
		Matrix SimulationToWorld;

		uint32 bEnabled					: 1;
		uint32 bKillOnDeactivate		: 1;
		uint32 bKillOnCompleted			: 1;
		uint32 bRequiresSorting			: 1;
		uint32 bHaltSpawning			: 1;
		uint32 bIgnoreComponentScale	: 1;
		int32 bRenderDataDirty			: 1;

		int32 SortMode;
		uint8* ParticleData;
		uint16* ParticleIndices;
		//uint8* InstanceData;

		//int32 InstancePayloadSize;
		//int32 PayloadOffset;
		int32 ParticleSize;
		int32 ParticleStride;
		int32 ActiveParticles;
		uint32 ParticleCounter;

		int32 MaxActiveParticles;
		float SpawnFraction;
		float SecondsSinceCreation;
		float EmitterTime;
		float LastDeltaTime;

		Box ParticleBoundingBox;
		int32 LoopCount;
		float EmitterDuration;
		float CurrentDelay;
		bool bEmitterIsDone;

		bool bUseLocalSpace = false;
		Vector Origin = Vector::ZeroVector;
		Quat Rotation = Quat::Identity;

		RandomStream EmitterRandomStream;

		std::vector<TRefCountPtr<ParticleModuleSpawnBase>> SpawningModules;
		std::vector<TRefCountPtr<ParticleModule>> SpawnModules;
		std::vector<TRefCountPtr<ParticleModule>> UpdateModules;

		virtual void InitParameters(ParticleEmitter* InTemplate, ParticleSystemComponent* InComponent);
		virtual void Init();
		World* GetWorld() const;

		void UpdateTransforms();

		virtual uint32 RequiredBytes();
		virtual bool Resize(int32 NewMaxActiveParticles);
		virtual void Tick(float DeltaTime);
		//void CheckEmitterFinished();
		
		virtual float Tick_EmitterTimeSetup(float DeltaTime);
		virtual float Tick_SpawnParticles(float DeltaTime, bool bFirstTime);
		virtual void Tick_ModuleUpdate(float DeltaTime);
		//virtual void Tick_ModulePostUpdate(float DeltaTime);
		//virtual void Tick_ModuleFinalUpdate(float DeltaTime);
		
		//virtual Box GetBoundingBox();
		//virtual void UpdateBoundingBox(float DeltaTime);
		
		//virtual uint32 CalculateParticleStride(uint32 ParticleSize);
		virtual void ResetParticleParameters(float DeltaTime);

		virtual float Spawn(float DeltaTime);
		void SpawnParticles( int32 Count, float StartTime, float Increment, const Vector& InitialLocation, const Vector& InitialVelocity );
		//virtual void ForceSpawn(float DeltaTime, int32 InSpawnCount, int32 InBurstCount, Vector& InLocation, Vector& InVelocity);
		//void CheckSpawnCount(int32 InNewCount, int32 InMaxCount);
		virtual void PreSpawn(BaseParticle* Particle, const Vector& InitialLocation, const Vector& InitialVelocity);
		virtual void PostSpawn(BaseParticle* Particle, float InterpolationPercentage, float SpawnTime);
		
		//virtual bool HasCompleted();
		virtual void KillParticles();
		virtual void KillParticle(int32 Index);
		//virtual void KillParticlesForced(bool bFireEvents = false);
		//
		//virtual void SetHaltSpawning(bool bInHaltSpawning)
		//{
		//	bHaltSpawning = bInHaltSpawning;
		//}
		//
		//virtual BaseParticle* GetParticle(int32 Index);
		//inline int32 GetParticleDirectIndex(int32 InIndex)
		//{
		//	if (InIndex < MaxActiveParticles)
		//	{
		//		return ParticleIndices[InIndex];
		//	}
		//	return -1;
		//}
		//virtual BaseParticle* GetParticleDirect(int32 InDirectIndex);
		//
		//void SetupEmitterDuration();
		//bool HasActiveParticles()
		//{
		//	return ActiveParticles > 0;
		//}
		//
		//virtual bool IsDynamicDataRequired();
		//
		//virtual int32 GetMeshRotationOffset() const
		//{
		//	return 0;
		//}
		//
		//virtual bool IsMeshRotationActive() const
		//{
		//	return false;
		//}
		//
		//virtual void OnEmitterInstanceKilled(ParticleEmitterInstance* Instance)
		//{
		//
		//}


	};
	
	class ParticleMeshEmitterInstance : public ParticleEmitterInstance
	{
	public:
		ParticleMeshEmitterInstance();
		virtual ~ParticleMeshEmitterInstance();

		bool bHasRotation : 1;
		int32 MeshRotationOffset;

		virtual void Tick(float DeltaTime) override;
		virtual void PostSpawn(BaseParticle* Particle, float InterpolationPercentage, float SpawnTime) override;

		virtual uint32 RequiredBytes() override;
		virtual bool Resize(int32 NewMaxActiveParticles) override;

		RandomStream RandStream;

	};

	// ---------------------------------------------------------------------------------------

#if WITH_EDITOR
	class ParticleStats
	{
	public:

		static void AddParticleCounter(int32 Amount) { ParticleCounter += Amount; }
		static void ResetParticleCounter() { ParticleCounter = 0; }

		inline static int32 GetParticleCounter() { return ParticleCounter; }

	private:
		static std::atomic<int32> ParticleCounter;
#endif
	};

}