#pragma once

#include "ForwardTypes.h"

namespace Drn
{
#define DECLARE_PARTICLE(Name,Address)		\
	BaseParticle& Name = *((BaseParticle*) (Address));

#define DECLARE_PARTICLE_CONST(Name,Address)		\
	const BaseParticle& Name = *((const BaseParticle*) (Address));

#define DECLARE_PARTICLE_PTR(Name,Address)		\
	BaseParticle* Name = (BaseParticle*) (Address);

#define SPAWN_INIT																										\
	drn_check((Owner != NULL) && (Owner->Component != NULL));															\
	const int32			ActiveParticles	= Owner->ActiveParticles;														\
	const uint32		ParticleStride	= Owner->ParticleStride;														\
	uint32				CurrentOffset	= Offset;																		\
	BaseParticle&	Particle			= *(ParticleBase);

#define PARTICLE_ELEMENT(Type,Name)																						\
	Type& Name = *((Type*)((uint8*)ParticleBase + CurrentOffset));														\
	CurrentOffset += sizeof(Type);

#define BEGIN_UPDATE_LOOP																								\
	{																													\
		drn_check((Owner) && (Owner->Component));																		\
		int32&				ActiveParticles = Owner->ActiveParticles;													\
		uint32				CurrentOffset	= Offset;																	\
		const uint8*		ParticleData	= Owner->ParticleData;														\
		const uint32		ParticleStride	= Owner->ParticleStride;													\
		uint16*				ParticleIndices	= Owner->ParticleIndices;													\
		for(int32 i=ActiveParticles-1; i>=0; i--)																		\
		{																												\
			const int32	CurrentIndex	= ParticleIndices[i];															\
			const uint8* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;									\
			BaseParticle& Particle		= *((BaseParticle*) ParticleBase);												\
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)															\
			{																											\

#define END_UPDATE_LOOP																									\
			}																											\
			CurrentOffset				= Offset;																		\
		}																												\
	}

#define CONTINUE_UPDATE_LOOP																							\
	CurrentOffset = Offset;																								\
	continue;

#define KILL_CURRENT_PARTICLE																							\
	{																													\
		ParticleIndices[i]					= ParticleIndices[ActiveParticles-1];										\
		ParticleIndices[ActiveParticles-1]	= CurrentIndex;																\
		ActiveParticles--;																								\
	}

	enum class EEmitterType : uint8
	{
		Sprite_Cpu,
		Sprite_Gpu,
		Mesh,
		Beam,
		Ribbon,
		AnimTrail
	};

	enum EParticleEventType
	{
		EPET_Any,
		EPET_Spawn,
		EPET_Death,
		EPET_Collision,
		EPET_Burst,
		EPET_Blueprint,
		EPET_MAX,
	};

	struct ParticleEventInstancePayload
	{
		uint32 bSpawnEventsPresent:1;
		uint32 bDeathEventsPresent:1;
		uint32 bCollisionEventsPresent:1;
		uint32 bBurstEventsPresent:1;

		int32 SpawnTrackingCount;
		int32 DeathTrackingCount;
		int32 CollisionTrackingCount;
		int32 BurstTrackingCount;
	};

	struct ParticleCollisionPayload
	{
		Vector	UsedDampingFactor;
		int32	UsedCollisions;
		Vector	UsedDampingFactorRotation;
		float	Delay;
	};

	enum EParticleStates
	{
		/** Ignore updates to the particle						*/
		STATE_Particle_JustSpawned			= 0x02000000,
		/** Ignore updates to the particle						*/
		STATE_Particle_Freeze				= 0x04000000,
		/** Ignore collision updates to the particle			*/
		STATE_Particle_IgnoreCollisions		= 0x08000000,
		/**	Stop translations of the particle					*/
		STATE_Particle_FreezeTranslation	= 0x10000000,
		/**	Stop rotations of the particle						*/
		STATE_Particle_FreezeRotation		= 0x20000000,
		/** Combination for a single check of 'ignore' flags	*/
		STATE_Particle_CollisionIgnoreCheck	= STATE_Particle_Freeze |STATE_Particle_IgnoreCollisions | STATE_Particle_FreezeTranslation| STATE_Particle_FreezeRotation,
		/** Delay collision updates to the particle				*/
		STATE_Particle_DelayCollisions		= 0x40000000,
		/** Flag indicating the particle has had at least one collision	*/
		STATE_Particle_CollisionHasOccurred	= 0x80000000,
		/** State mask. */
		STATE_Mask = 0xFE000000,
		/** Counter mask. */
		STATE_CounterMask = (~STATE_Mask)
	};

	struct BaseParticle
	{
		// 16 bytes
		Vector			OldLocation;			// Last frame's location, used for collision
		float			RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death)

		// 16 bytes
		Vector			Location;				// Current location
		float			OneOverMaxLifetime;		// Reciprocal of lifetime

		// 16 bytes
		Vector			BaseVelocity;			// Velocity = BaseVelocity at the start of each frame.
		float			Rotation;				// Rotation of particle (in Radians)

		// 16 bytes
		Vector			Velocity;				// Current velocity, gets reset to BaseVelocity each frame to allow 
		float			BaseRotationRate;		// Initial angular velocity of particle (in Radians per second)

		// 16 bytes
		Vector			BaseSize;				// Size = BaseSize at the start of each frame
		float			RotationRate;			// Current rotation rate, gets reset to BaseRotationRate each frame

		// 16 bytes
		Vector			Size;					// Current size, gets reset to BaseSize each frame
		int32			Flags;					// Flags indicating various particle states

		// 16 bytes
		Vector4			Color;					// Current color of particle.

		// 16 bytes
		Vector4			BaseColor;				// Base color of the particle
	};

	struct MeshRotationPayloadData
	{
		Vector	InitialOrientation;		// from mesh data module
		Vector  InitRotation;				// from init rotation module
		Vector  Rotation;
		Vector	CurContinuousRotation;
		Vector  RotationRate;
		Vector  RotationRateBase;
	};

	struct MeshParticleInstanceVertex
	{
		/** The color of the particle. */
		Vector4 Color;

		/** The instance to world transform of the particle. Translation vector is packed into W components. */
		Vector4 Transform[3];

		/** The velocity of the particle, XYZ: direction, W: speed. */
		Vector4 Velocity;

		/** The sub-image texture offsets for the particle. */
		int16 SubUVParams[4];

		/** The sub-image lerp value for the particle. */
		float SubUVLerp;

		/** The relative time of the particle. */
		float RelativeTime;
	};

	struct MeshParticleInstanceVertexDynamicParameter
	{
		/** The dynamic parameter of the particle. */
		float DynamicValue[4];
	};

	struct MeshParticleInstanceVertexPrevTransform
	{
		Vector4 PrevTransform0;
		Vector4 PrevTransform1;
		Vector4 PrevTransform2;
	};

	struct ParticleSpawnPerUnitInstancePayload
	{
		float	CurrentDistanceTravelled;
	};


	struct ParticleBurst
	{
		int32 Count;
		int32 CountLow;
		float Time;

		ParticleBurst() : Count(0), CountLow(-1), Time(0.0f) {}

		friend Archive& operator<<(Archive& Ar, ParticleBurst& Data);
		friend Archive& operator>>(Archive& Ar, ParticleBurst& Data);

#if WITH_EDITOR
		bool Draw();
#endif
	};

	struct ParticleEventData
	{
		int32 Type;
		std::string EventName;
		float EmitterTime;
		Vector Location;
		Vector Velocity;

		//TArray<class UParticleModuleEventSendToGame*> EventData;

		ParticleEventData()
			: Type(0)
			, EmitterTime(0)
		{}
	};

	struct ParticleExistingData : ParticleEventData
	{
		float ParticleTime;
		Vector Direction;

		ParticleExistingData()
			: ParticleTime(0)
			, Direction(Vector::ZeroVector)
		{}
	};

	struct ParticleEventSpawnData : public ParticleEventData
	{
	};

	struct ParticleEventDeathData : public ParticleExistingData
	{

	};

	struct ParticleEventCollideData : public ParticleExistingData
	{
		Vector Normal;
		float Time;
		int32 Item;
		std::string BoneName;
		class PhysicalMaterial* PhysMat;

		ParticleEventCollideData()
			: Normal(Vector::ZeroVector)
			, Time(0)
			, Item(0)
		{}

	};

	struct ParticleEventBurstData : public ParticleEventData
	{
		int32 ParticleCount;

		ParticleEventBurstData()
			: ParticleCount(0)
		{}
	};

	struct ParticleEventKismetData : public ParticleEventData
	{
	};

	struct ParticleModuleMetaData
	{
		ParticleModuleMetaData(const std::string InDisplayName)
			: DisplayName(InDisplayName)
		{}

		ParticleModuleMetaData() : ParticleModuleMetaData("Invalid")
		{}

		std::string DisplayName;
	};

	struct ParticleModuleCategory
	{
		ParticleModuleCategory(const std::string& InCategoryName)
			: CategoryName(InCategoryName)
		{}

		std::string CategoryName;
		std::vector<EParticleModule> Modules;
	};

	class ParticleTypes
	{
	public:
		static void RegisterParticleModules();

		template<typename T>
		static void RegisterParticleModule(EParticleModule Module, const std::string& DisplayName, const std::string& CategoryName);

		static std::function<ParticleModule*()> ParticleModuleFactory[(int32)EParticleModule::Max];
		static inline ParticleModule* CreateParticleModule(EParticleModule Module) { return ParticleModuleFactory[int32(Module)](); }

#if WITH_EDITOR

		static ParticleModuleMetaData ParticleModulesMetaData[(int32)EParticleModule::Max];
		static std::vector<ParticleModuleCategory> ParticleModuleCategories;
#endif
	};

}