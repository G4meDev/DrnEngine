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
	drn_check((Owner != NULL) && (Owner->Component != NULL));																\
	const int32			ActiveParticles	= Owner->ActiveParticles;															\
	const uint32		ParticleStride	= Owner->ParticleStride;															\
	BaseParticle&	Particle			= *(ParticleBase);

	enum class EEmitterType : uint8
	{
		Sprite_Cpu,
		Sprite_Gpu,
		Mesh,
		Beam,
		Ribbon,
		AnimTrail
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


}