#include "DrnPCH.h"
#include "ParticleModuleCollision.h"
#include "Runtime/Particle/ParticleModuleEventGenerator.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleCollision::ParticleModuleCollision()
		: ParticleModule()
		, DampingFactor(new ParticleDistributionVectorConstant(Vector::ZeroVector))
		, DampingFactorRotation(new ParticleDistributionVectorConstant(Vector::OneVector))
		, MaxCollisions(new ParticleDistributionFloatConstant(1.0f))
		, ParticleMass(new ParticleDistributionFloatConstant(0.1f))
		, DelayAmount(new ParticleDistributionFloatConstant(0.0f))
		, bApplyPhysics(false)
		, bIgnoreSourceActor(true)
		, bOnlyVerticalNormalsDecrementCount(false)
		, MaxCollisionDistance(10.0f)
		, DirScalar(3.5f)
		, VerticalFudgeFactor(0.1f)
		, CollisionCompletionOption(EParticleCollisionComplete::EPCC_Kill)
	{
		bSpawnModule = true;
		bUpdateModule = true;

		CollisionTypes.push_back(ECollisionChannel::ECC_WorldStatic);
	}

	void ParticleModuleCollision::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			DampingFactor = ParticleDistributionVector::Create(Ar);
			DampingFactorRotation = ParticleDistributionVector::Create(Ar);

			ParticleMass = ParticleDistributionFloat::Create(Ar);
			MaxCollisions = ParticleDistributionFloat::Create(Ar);
			DelayAmount = ParticleDistributionFloat::Create(Ar);

			Ar >> *(uint32*)&CollisionCompletionOption;

			Ar >> bApplyPhysics;
			Ar >> bIgnoreSourceActor;
			Ar >> bOnlyVerticalNormalsDecrementCount;

			Ar >> MaxCollisionDistance;
			Ar >> DirScalar;
			Ar >> VerticalFudgeFactor;

			uint8 CollisionTypesCount = 0;
			Ar >> CollisionTypesCount;
			CollisionTypes.resize(CollisionTypesCount);
			for (int32 CollisionIndex = 0; CollisionIndex < CollisionTypesCount; CollisionIndex++)
			{
				Ar >> *(uint32*)&CollisionTypes[CollisionIndex];
			}
		}
		else
		{
			DampingFactor->Serialize(Ar);
			DampingFactorRotation->Serialize(Ar);

			ParticleMass->Serialize(Ar);
			MaxCollisions->Serialize(Ar);
			DelayAmount->Serialize(Ar);

			Ar << (uint32)CollisionCompletionOption;

			Ar << bApplyPhysics;
			Ar << bIgnoreSourceActor;
			Ar << bOnlyVerticalNormalsDecrementCount;

			Ar << MaxCollisionDistance;
			Ar << DirScalar;
			Ar << VerticalFudgeFactor;

			const uint8 CollisionTypesCount = CollisionTypes.size();
			Ar << CollisionTypesCount;
			for (int32 CollisionIndex = 0; CollisionIndex < CollisionTypesCount; CollisionIndex++)
			{
				Ar << (uint32)CollisionTypes[CollisionIndex];
			}
		}
	}

	uint32 ParticleModuleCollision::RequiredBytes()
	{
		return sizeof(ParticleCollisionPayload);
	}

	uint32 ParticleModuleCollision::RequiredBytesPerInstance()
	{
		return 0;
	}

	uint32 ParticleModuleCollision::PrepPerInstanceBlock( ParticleEmitterInstance* Owner, void* InstData )
	{
		return 0;
	}

	void ParticleModuleCollision::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SCOPE_STAT("ParticleCollisionTime");
		SPAWN_INIT;
		{
			PARTICLE_ELEMENT(ParticleCollisionPayload, CollisionPayload);
			CollisionPayload.UsedDampingFactor = DampingFactor->GetValue(Owner->EmitterTime, Owner);
			CollisionPayload.UsedDampingFactorRotation = DampingFactorRotation->GetValue(Owner->EmitterTime, Owner);
			CollisionPayload.UsedCollisions = Math::RoundToInt(MaxCollisions->GetValue(Owner->EmitterTime, Owner));
			CollisionPayload.Delay = DelayAmount->GetValue(Owner->EmitterTime, Owner);
			if (CollisionPayload.Delay > SpawnTime)
			{
				Particle.Flags |= STATE_Particle_DelayCollisions;
				Particle.Flags &= ~STATE_Particle_CollisionHasOccurred;
			}
		}
	}

	void ParticleModuleCollision::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		SCOPE_STAT("ParticleCollisionTime");
		drn_check(Owner);
		drn_check(Owner->Component);
		World* OwningWorld = Owner->Component->GetWorld();
		if (!Owner->HasActiveParticles())
		{
			return;
		}

		Actor* OwningActor = Owner->Component->GetOwningActor();
		drn_check(Owner->Emitter);

		const int32 MeshRotationOffset = Owner->Emitter->GetMeshRotationOffset();
		const bool bMeshRotationActive = Owner->Emitter->IsMeshRotationActive();

		const Transform& OwnerTM = Owner->Component->GetWorldTransform();
		const Vector ParentScale = OwnerTM.GetScale();

		ParticleEventInstancePayload* EventPayload = nullptr;
		if (Owner->Emitter->EventGenerator)
		{
			EventPayload = (ParticleEventInstancePayload*)(Owner->GetModuleInstanceData(Owner->Emitter->EventGenerator));
			if (EventPayload && 
				(EventPayload->bCollisionEventsPresent == false) && 
				(EventPayload->bDeathEventsPresent == false))
			{
				EventPayload = nullptr;
			}
		}

		float SquaredMaxCollisionDistance = MaxCollisionDistance * MaxCollisionDistance;
		BEGIN_UPDATE_LOOP;
		{
			if ((Particle.Flags & STATE_Particle_CollisionIgnoreCheck) != 0)
			{
				CONTINUE_UPDATE_LOOP;
			}

			PARTICLE_ELEMENT(ParticleCollisionPayload, CollisionPayload);
			if ((Particle.Flags & STATE_Particle_DelayCollisions) != 0)
			{
				if (CollisionPayload.Delay > Particle.RelativeTime)
				{
					CONTINUE_UPDATE_LOOP;
				}
				Particle.Flags &= ~STATE_Particle_DelayCollisions;
			}

			Vector Location;
			Vector OldLocation;

			Location = Particle.Location + Particle.Velocity * DeltaTime;
			if (Owner->Emitter->bUseLocalSpace)
			{
				// Transform the location and old location into world space
				Location		= OwnerTM.TransformPosition(Location);
				OldLocation		= OwnerTM.TransformPosition(Particle.OldLocation);
			}
			else
			{
				OldLocation	= Particle.OldLocation;
			}
			Vector	Direction = (Location - OldLocation).GetSafeNormal();

			Vector Size = Particle.Size * ParentScale;
			Vector	Extent(0.0f);

			//UParticleModuleTypeDataMesh* MeshType = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
			//if (MeshType && MeshType->Mesh)
			//{
			//	Extent = MeshType->Mesh->GetBounds().BoxExtent;
			//	Extent = MeshType->bCollisionsConsiderPartilceSize ? Extent * Size : Extent;
			//}
		
			HitResult Hit;
			Hit.Normal = Vector::ZeroVector;
			drn_check( Owner->Component );

			Vector End = Location + Direction * Size / DirScalar;

			if (OwningWorld->IsGameWorld())
			{
				const bool bCloseEnough = (OwningWorld->GetPlayerWorldView().Location - End).SizeSquared() < SquaredMaxCollisionDistance;
				if (bCloseEnough == false)
				{
					Particle.Flags |= STATE_Particle_IgnoreCollisions;
					CONTINUE_UPDATE_LOOP;
				}
			}

			Actor* IgnoreActor = bIgnoreSourceActor ? OwningActor : NULL;

			if (PerformCollisionCheck(Owner, &Particle, Hit, IgnoreActor, End, OldLocation, Extent))
			{
				bool bDecrementMaxCount = true;

				if (bOnlyVerticalNormalsDecrementCount)
				{
					if ((Hit.Normal.IsNearlyZero() == false) && (Math::Abs(Hit.Normal.Z) + VerticalFudgeFactor) < 1.0f)
					{
						bDecrementMaxCount = false;
					}
				}

				if (bDecrementMaxCount)
				{
					CollisionPayload.UsedCollisions--;
				}

				if (CollisionPayload.UsedCollisions > 0)
				{
					if (Owner->Emitter->bUseLocalSpace)
					{
						// Transform the particle velocity to world space
						Vector OldVelocity		= OwnerTM.TransformVector(Particle.Velocity);
						Vector BaseVelocity		= OwnerTM.TransformVector(Particle.BaseVelocity);
						BaseVelocity			= BaseVelocity.MirrorByVector(Hit.Normal) * CollisionPayload.UsedDampingFactor;

						Particle.BaseVelocity		= OwnerTM.InverseTransformVector(BaseVelocity);
						Particle.BaseRotationRate	= Particle.BaseRotationRate * CollisionPayload.UsedDampingFactorRotation.X;
						if (bMeshRotationActive && MeshRotationOffset > 0)
						{
							MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
							PayloadData->RotationRateBase *= CollisionPayload.UsedDampingFactorRotation;
						}

						Vector NewVelocity	= Direction.MirrorByVector(Hit.Normal) * (Location - OldLocation).Length() * CollisionPayload.UsedDampingFactor;
						Particle.Velocity		= Vector::ZeroVector;

						// New location
						Vector	NewLocation		= Location + NewVelocity * (1.f - Hit.Time);
						Particle.Location		= OwnerTM.InverseTransformPosition(NewLocation);

						//if (bApplyPhysics)
						//{
						//	PrimitiveComponent* PrimitiveComponent = Hit.HitComponent;
						//	if(PrimitiveComponent && PrimitiveComponent->IsAnySimulatingPhysics())
						//	{
						//		Vector vImpulse;
						//		vImpulse = (NewVelocity - OldVelocity) * -1 * ParticleMass->GetValue(Particle.RelativeTime, Owner);
						//		PrimitiveComponent->AddImpulseAtLocation(vImpulse, Hit.Location, Hit.BoneName);
						//	}
						//}
					}
					else
					{
						Vector vOldVelocity = Particle.Velocity;

						// Reflect base velocity and apply damping factor.
						Particle.BaseVelocity		= Particle.BaseVelocity.MirrorByVector(Hit.Normal) * CollisionPayload.UsedDampingFactor;
						Particle.BaseRotationRate	= Particle.BaseRotationRate * CollisionPayload.UsedDampingFactorRotation.X;
						if (bMeshRotationActive && MeshRotationOffset > 0)
						{
							MeshRotationPayloadData* PayloadData = (MeshRotationPayloadData*)((uint8*)&Particle + MeshRotationOffset);
							PayloadData->RotationRateBase *= CollisionPayload.UsedDampingFactorRotation;
						}

						// Reset the current velocity and manually adjust location to bounce off based on normal and time of collision.
						Vector vNewVelocity	= Direction.MirrorByVector(Hit.Normal) * (Location - OldLocation).Length() * CollisionPayload.UsedDampingFactor;
						Particle.Velocity		= Vector::ZeroVector;
						Particle.Location	   += vNewVelocity * (1.f - Hit.Time);

						//if (bApplyPhysics)
						//{
						//	PrimitiveComponent* PrimitiveComponent = Hit.HitComponent;
						//	if(PrimitiveComponent && PrimitiveComponent->IsAnySimulatingPhysics())
						//	{
						//		Vector vImpulse;
						//		vImpulse = (vNewVelocity - vOldVelocity) * -1 * ParticleMass->GetValue(Particle.RelativeTime, Owner);
						//		PrimitiveComponent->AddImpulseAtLocation(vImpulse, Hit.Location, Hit.BoneName);
						//	}
						//}
					}

					if (EventPayload && EventPayload->bCollisionEventsPresent)
					{
						Owner->Emitter->EventGenerator->HandleParticleCollision(Owner, EventPayload, &CollisionPayload, &Hit, &Particle, Direction);
					}
				}
				else
				{
					if (Owner->Emitter->bUseLocalSpace == true)
					{
						Size = OwnerTM.TransformVector(Size);
					}
					Particle.Location = Hit.Location;
					if (Owner->Emitter->bUseLocalSpace == true)
					{
						// We need to transform the location back relative to the PSys.
						// NOTE: LocalSpace makes sense only for stationary emitters that use collision.
						Particle.Location = OwnerTM.InverseTransformPosition(Particle.Location);
					}
					switch (CollisionCompletionOption)
					{
					case EPCC_Kill:
						{
							if (EventPayload && (EventPayload->bDeathEventsPresent == true))
							{
								Owner->Emitter->EventGenerator->HandleParticleKilled(Owner, EventPayload, &Particle);
							}
							KILL_CURRENT_PARTICLE;
						}
						break;
					case EPCC_Freeze:
						{
							Particle.Flags |= STATE_Particle_Freeze;
						}
						break;
					case EPCC_HaltCollisions:
						{
							Particle.Flags |= STATE_Particle_IgnoreCollisions;
						}
						break;
					case EPCC_FreezeTranslation:
						{
							Particle.Flags |= STATE_Particle_FreezeTranslation;
						}
						break;
					case EPCC_FreezeRotation:
						{
							Particle.Flags |= STATE_Particle_FreezeRotation;
						}
						break;
					case EPCC_FreezeMovement:
						{
							Particle.Flags |= STATE_Particle_FreezeRotation;
							Particle.Flags |= STATE_Particle_FreezeTranslation;
						}
						break;
					}

					if (EventPayload && (EventPayload->bCollisionEventsPresent == true))
					{
						Owner->Emitter->EventGenerator->HandleParticleCollision(Owner, EventPayload, &CollisionPayload, &Hit, &Particle, Direction);
					}
				}
				Particle.Flags |= STATE_Particle_CollisionHasOccurred;
			}
		}
		END_UPDATE_LOOP;
	}

	bool ParticleModuleCollision::PerformCollisionCheck( ParticleEmitterInstance* Owner, BaseParticle* InParticle, HitResult& Hit, Actor* SourceActor, const Vector& End,
		const Vector& Start, const Vector& Extent )
	{
		drn_check(Owner && Owner->Component);
		return Owner->Component->ParticleLineCheck(Hit, SourceActor, End, Start, Extent, CollisionTypes);
	}

#if WITH_EDITOR
	bool ParticleModuleCollision::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);

		{
			const char* const Options[] = { "Kill", "Freeze", "HaltCollisions", "FreezeTranslation", "FreezeRotation", "FreezeMovement" };
			int32 Selected = CollisionCompletionOption;
			bDirty |= ImGui::Combo("Collision Completion Option", &Selected, Options, _countof(Options));
			if (bDirty)
			{
				CollisionCompletionOption = (EParticleCollisionComplete)Selected;
			}
		}

		{
			if (ImGui::Button("Add Channel Type"))
			{
				CollisionTypes.push_back({});
				bDirty = true;
			}

			if (ImGui::Button("Clear Channel Type"))
			{
				CollisionTypes.clear();
				bDirty = true;
			}

			for (int32 CollisionIndex = 0; CollisionIndex < CollisionTypes.size(); CollisionIndex++)
			{
				ImGui::PushID(CollisionIndex);
				DrawCollisionObjectType("Type", CollisionTypes[CollisionIndex]);
				ImGui::PopID();
			}
		}

		bDirty |= DampingFactor->Draw(DampingFactor, "Damping Factor");
		bDirty |= DampingFactorRotation->Draw(DampingFactorRotation, "Damping Factor Rotation");

		bDirty |= ParticleMass->Draw(ParticleMass, "Particle Mass");
		bDirty |= MaxCollisions->Draw(MaxCollisions, "Max Collisions");
		bDirty |= DelayAmount->Draw(DelayAmount, "Delay Amount");

		bDirty |= ImGui::Checkbox("Apply Physics", &bApplyPhysics);
		bDirty |= ImGui::Checkbox("Ignore Source Actor", &bIgnoreSourceActor);
		bDirty |= ImGui::Checkbox("Only Vertical Normals Decrement Count", &bOnlyVerticalNormalsDecrementCount);

		bDirty |= ImGui::InputFloat("Max Collision Distance", &MaxCollisionDistance);
		bDirty |= ImGui::InputFloat("Dir Scalar", &DirScalar);
		bDirty |= ImGui::InputFloat("Vertical Fudge Factor", &VerticalFudgeFactor);

		return bDirty;
	}
#endif
}  // namespace Drn