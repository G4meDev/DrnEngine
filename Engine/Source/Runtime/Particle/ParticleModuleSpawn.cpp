#include "DrnPCH.h"
#include "ParticleModuleSpawn.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleSpawn::ParticleModuleSpawn()
		: ParticleModuleSpawnBase()
	{
		ParticleDistributionFloatConstant* InitalSpawnRate = new ParticleDistributionFloatConstant();
		InitalSpawnRate->Constant = 5.0f;
		SpawnRate = InitalSpawnRate;
	}

	uint32 ParticleModuleSpawn::RequiredBytesPerInstance()
	{
		return BurstList.size();
	}

	bool ParticleModuleSpawn::GetSpawnAmount( ParticleEmitterInstance* Owner,
		float OldLeftover, float DeltaTime, int32& Number, float& Rate)
	{
		drn_check(Owner);
		drn_check(SpawnRate);

		Rate = SpawnRate->GetValue(Owner, &GetRandomStream(Owner));
		return true;
	}

	bool ParticleModuleSpawn::GetBurstCount( ParticleEmitterInstance* Owner, float OldLeftover, float DeltaTime, int32& Number )
	{
		Number = 0;
		float SpawnRateInc = 0.0f;

		uint8* InstData = Owner->GetModuleInstanceData(this);
		if (BurstList.size() > 0 && InstData)
		{
			RandomStream& RStream = GetRandomStream(Owner);

			for (int32 BurstIdx = 0; BurstIdx < BurstList.size(); BurstIdx++)
			{
				ParticleBurst* BurstEntry = &(BurstList[BurstIdx]);
				uint8& LocalBurstFired = InstData[BurstIdx];

				if (!LocalBurstFired && Owner->EmitterTime >= BurstEntry->Time)
				{
					if (DeltaTime < 0.00001f)
					{
						DeltaTime = 0.00001f;
					}
					int32 Count = BurstEntry->Count;
					if (BurstEntry->CountLow > -1)
					{
						Count = RStream.RandRange(BurstEntry->CountLow, BurstEntry->Count);
					}
					SpawnRateInc += Count / DeltaTime;
					Number += Count;
					LocalBurstFired = true;
				}
			}
		}

		return SpawnRateInc;
	}

	bool ParticleModuleSpawn::CheckFinished( ParticleEmitterInstance* Owner )
	{
		drn_check(SpawnRate);

		const bool bBurstFinished = BurstList.size() > 0 ? BurstList.back().Time < Owner->EmitterTime : true;
		float MinSpawnRate = 0.0f;
		float MaxSpawnRate = 0.0f;
		SpawnRate->GetOutRange(MinSpawnRate, MaxSpawnRate);
		return (MaxSpawnRate == 0.0f) && bBurstFinished;
	}

	void ParticleModuleSpawn::ResetBurstList(ParticleEmitterInstance* Owner)
	{
		uint8* InstData = Owner->GetModuleInstanceData(this);
		if (BurstList.size() > 0 && InstData)
		{
			for (int32 BurstIdx = 0; BurstIdx < BurstList.size(); BurstIdx++)
			{
				InstData[BurstIdx] = false;
			}
		}
	}

	void ParticleModuleSpawn::Serialize( Archive& Ar )
	{
		ParticleModuleSpawnBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			SpawnRate = ParticleDistributionFloat::Create(Ar);

			uint8 BurstCount;
			Ar >> BurstCount;
			BurstList.resize(BurstCount);

			for (int32 i = 0; i < BurstCount; i++)
			{
				Ar >> BurstList[i];
			}
		}

		else
		{
			SpawnRate->Serialize(Ar);

			const uint8 BurstCount = (uint8)BurstList.size();
			Ar << BurstCount;

			for (int32 i = 0; i < BurstCount; i++)
			{
				Ar << BurstList[i];
			}
		}
	}

#if WITH_EDITOR
	bool ParticleModuleSpawn::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleSpawnBase::Draw(Owner);

		if (SpawnRate && ImGui::CollapsingHeader("Spawn Rate", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= SpawnRate->Draw(SpawnRate);
		}

		if (ImGui::Button("Add Burst"))
		{
			BurstList.push_back({});
			bDirty = true;
		}
		if (ImGui::Button("Remove Burst"))
		{
			BurstList.pop_back();
			bDirty = true;
		}

		for (ParticleBurst& Burst : BurstList)
		{
			ImGui::PushID(&Burst);
			bDirty |= Burst.Draw();
			ImGui::PopID();
		}

		return bDirty;
	}
#endif

// --------------------------------------------------------------------------------------------------------------------

	ParticleModuleSpawnPerUnit::ParticleModuleSpawnPerUnit()
		: ParticleModuleSpawnBase()
		, UnitScalar(5.0f)
		, MovementTolerance(0.1f)
		, MaxFrameDistance(0.0f)
		, bIgnoreSpawnRateWhenMoving(false)
		, bIgnoreMovementAlongX(false)
		, bIgnoreMovementAlongY(false)
		, bIgnoreMovementAlongZ(false)
	{
		ParticleDistributionFloatConstant* InitalSpawnPerUnit = new ParticleDistributionFloatConstant();
		InitalSpawnPerUnit->Constant = 1.0f;
		SpawnPerUnit = InitalSpawnPerUnit;
	}

	void ParticleModuleSpawnPerUnit::Serialize( Archive& Ar )
	{
		ParticleModuleSpawnBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			SpawnPerUnit = ParticleDistributionFloat::Create(Ar);
			Ar >> UnitScalar;
			Ar >> MovementTolerance;
			Ar >> MaxFrameDistance;
			Ar >> bIgnoreSpawnRateWhenMoving;
			Ar >> bIgnoreMovementAlongX;
			Ar >> bIgnoreMovementAlongY;
			Ar >> bIgnoreMovementAlongZ;
		}

		else
		{
			SpawnPerUnit->Serialize(Ar);
			Ar << UnitScalar;
			Ar << MovementTolerance;
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

	bool ParticleModuleSpawnPerUnit::GetSpawnAmount( ParticleEmitterInstance* Owner,
		float OldLeftover, float DeltaTime, int32& Number, float& Rate )
	{
		drn_check(Owner);
		drn_check(SpawnPerUnit);

		bool bMoved = false;
		ParticleSpawnPerUnitInstancePayload* SPUPayload = NULL;
		float NewTravelLeftover = 0.0f;

		float ParticlesPerUnit = SpawnPerUnit->GetValue(Owner) / UnitScalar;

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

	bool ParticleModuleSpawnPerUnit::CheckFinished( ParticleEmitterInstance* Owner )
	{
		return true;
	}

#if WITH_EDITOR
	bool ParticleModuleSpawnPerUnit::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleSpawnBase::Draw(Owner);

		bDirty |= ImGui::InputFloat("Unit Scalar", &UnitScalar);
		bDirty |= ImGui::InputFloat("Movement Tolerance", &MovementTolerance);

		if (SpawnPerUnit && ImGui::CollapsingHeader("Spawn Per Unit", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bDirty |= SpawnPerUnit->Draw(SpawnPerUnit);
		}

		bDirty |= ImGui::InputFloat("Max Frame Distance", &MaxFrameDistance);
		bDirty |= ImGui::Checkbox("Ignore Spawn Rate When Moving", &bIgnoreSpawnRateWhenMoving);
		bDirty |= ImGui::Checkbox("Ignore Movement Along X", &bIgnoreMovementAlongX);
		bDirty |= ImGui::Checkbox("Ignore Movement Along Y", &bIgnoreMovementAlongY);
		bDirty |= ImGui::Checkbox("Ignore Movement Along Z", &bIgnoreMovementAlongZ);

		return bDirty;
	}
#endif

        }  // namespace Drn