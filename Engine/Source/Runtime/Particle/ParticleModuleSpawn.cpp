#include "DrnPCH.h"
#include "ParticleModuleSpawn.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	bool ParticleModuleSpawn::GetSpawnAmount(ParticleEmitterInstance* Owner, int32 Offset,
		float OldLeftover, float DeltaTime, int32& Number, float& Rate)
	{
		drn_check(Owner);

		Rate = SpawnRate;
		return true;
	}

	void ParticleModuleSpawn::Serialize( Archive& Ar )
	{
		ParticleModuleSpawnBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> SpawnRate;
		}

		else
		{
			Ar << SpawnRate;
		}
	}

#if WITH_EDITOR
	bool ParticleModuleSpawn::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleSpawnBase::Draw(Owner);

		bDirty |= ImGui::InputFloat("Spawn Rate", &SpawnRate);

		return bDirty;
	}
#endif
	
// --------------------------------------------------------------------------------------------------------------------

	void ParticleModuleSpawnPerUnit::Serialize( Archive& Ar )
	{
		ParticleModuleSpawnBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> UnitScalar;
			Ar >> MovementTolerance;
			Ar >> SpawnPerUnit;
			Ar >> MaxFrameDistance;
			Ar >> bIgnoreSpawnRateWhenMoving;
			Ar >> bIgnoreMovementAlongX;
			Ar >> bIgnoreMovementAlongY;
			Ar >> bIgnoreMovementAlongZ;
		}

		else
		{
			Ar << UnitScalar;
			Ar << MovementTolerance;
			Ar << SpawnPerUnit;
			Ar << MaxFrameDistance;
			Ar << bIgnoreSpawnRateWhenMoving;
			Ar << bIgnoreMovementAlongX;
			Ar << bIgnoreMovementAlongY;
			Ar << bIgnoreMovementAlongZ;
		}
	}

	uint32 ParticleModuleSpawnPerUnit::RequiredBytesPerInstance()
	{
		return sizeof(ParticleSpawnPerUnitInstancePayload);
	}

	bool ParticleModuleSpawnPerUnit::GetSpawnAmount( ParticleEmitterInstance* Owner, int32 Offset,
		float OldLeftover, float DeltaTime, int32& Number, float& Rate )
	{
		drn_check(Owner);

		bool bMoved = false;
		ParticleSpawnPerUnitInstancePayload* SPUPayload = NULL;
		float NewTravelLeftover = 0.0f;

		//float ParticlesPerUnit = SpawnPerUnit.GetValue(Owner->EmitterTime, Owner->Component) / UnitScalar;
		float ParticlesPerUnit = SpawnPerUnit / UnitScalar;

		if (ParticlesPerUnit >= 0.0f)
		{
			float LeftoverTravel = 0.0f;
			uint8* InstData = Owner->GetModuleInstanceData(this);
			if (InstData)
			{
				SPUPayload = (ParticleSpawnPerUnitInstancePayload*)InstData;
				LeftoverTravel = SPUPayload->CurrentDistanceTravelled;
			}

			Vector TravelDirection = Owner->Location - Owner->OldLocation;
			Vector RemoveComponentMultiplier(
				bIgnoreMovementAlongX ? 0.0f : 1.0f,
				bIgnoreMovementAlongY ? 0.0f : 1.0f,
				bIgnoreMovementAlongZ ? 0.0f : 1.0f
				);
			TravelDirection = TravelDirection * RemoveComponentMultiplier;

			float TravelDistance = TravelDirection.Length();
			if (MaxFrameDistance > 0.0f)
			{
				if (TravelDistance > MaxFrameDistance)
				{
					TravelDistance = 0.0f;
					if ( SPUPayload )
					{
						SPUPayload->CurrentDistanceTravelled = 0.0f;
					}
				}
			}

			if (TravelDistance > 0.0f)
			{
				if (TravelDistance > (MovementTolerance * UnitScalar))
				{
					bMoved = true;
				}

				TravelDirection = TravelDirection.GetSafeNormal();

				float NewLeftover = (TravelDistance + LeftoverTravel) * ParticlesPerUnit;
				Number = std::floor(NewLeftover);
				float InvDeltaTime = (DeltaTime > 0.0f) ? 1.0f / DeltaTime : 0.0f;
				Rate = Number * InvDeltaTime;
				NewTravelLeftover = (TravelDistance + LeftoverTravel) - (Number * UnitScalar);
				if (SPUPayload)
				{
					SPUPayload->CurrentDistanceTravelled = std::max<float>(0.0f, NewTravelLeftover);
				}

			}
			else
			{
				Number = 0;
				Rate = 0.0f;
			}
		}
		else
		{
			Number = 0;
			Rate = 0.0f;
		}

		if (bIgnoreSpawnRateWhenMoving == true)
		{
			if (bMoved == true)
			{
				return false;
			}
			return true;
		}

		//return bProcessSpawnRate;
		return true;
	}

#if WITH_EDITOR
	bool ParticleModuleSpawnPerUnit::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleSpawnBase::Draw(Owner);

		bDirty |= ImGui::InputFloat("Unit Scalar", &UnitScalar);
		bDirty |= ImGui::InputFloat("Movement Tolerance", &MovementTolerance);
		bDirty |= ImGui::InputFloat("Spawn Per Unit", &SpawnPerUnit);
		bDirty |= ImGui::InputFloat("Max Frame Distance", &MaxFrameDistance);
		bDirty |= ImGui::Checkbox("Ignore Spawn Rate When Moving", &bIgnoreSpawnRateWhenMoving);
		bDirty |= ImGui::Checkbox("Ignore Movement Along X", &bIgnoreMovementAlongX);
		bDirty |= ImGui::Checkbox("Ignore Movement Along Y", &bIgnoreMovementAlongY);
		bDirty |= ImGui::Checkbox("Ignore Movement Along Z", &bIgnoreMovementAlongZ);

		return bDirty;
	}
#endif

        }  // namespace Drn