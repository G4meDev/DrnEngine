#include "DrnPch.h"
#include "ParticleModuleSize.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{
	ParticleModuleSize::ParticleModuleSize()
		: ParticleModule()
		, StartSize(new ParticleDistributionVectorConstant(1.0f))
	{
		bSpawnModule = true;
	}

	void ParticleModuleSize::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		Vector Size		 = StartSize->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
		Particle.Size	+= Size;

		//AdjustParticleBaseSizeForUVFlipping(Size, Owner->CurrentLODLevel->RequiredModule->UVFlippingMode, *InRandomStream);
		Particle.BaseSize += Size;
	}

	void ParticleModuleSize::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartSize = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			StartSize->Serialize(Ar);
		}
	}


// -------------------------------------------------------------------------------------

	ParticleModuleSizeScale::ParticleModuleSizeScale()
		: ParticleModule()
		, SizeScale(new ParticleDistributionVectorConstant(1.0f))
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleSizeScale::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			SizeScale = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			SizeScale->Serialize(Ar);
		}
	}

	void ParticleModuleSizeScale::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		Vector ScaleFactor = SizeScale->GetValue(Particle.RelativeTime, Owner);
		Particle.Size = Particle.BaseSize * ScaleFactor;
	}

	void ParticleModuleSizeScale::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		BEGIN_UPDATE_LOOP;
			Vector ScaleFactor = SizeScale->GetValue(Particle.RelativeTime, Owner);
			Particle.Size = Particle.BaseSize * ScaleFactor;
		END_UPDATE_LOOP;
	}

// -------------------------------------------------------------------------------------

	ParticleModuleSizeByLife::ParticleModuleSizeByLife()
		: ParticleModule()
		, LifeMultiplier(new ParticleDistributionVectorConstant(1.0f))
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleSizeByLife::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			LifeMultiplier = ParticleDistributionVector::Create(Ar);
		}
		else
		{
			LifeMultiplier->Serialize(Ar);
		}
	}

	void ParticleModuleSizeByLife::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		Vector SizeScale = LifeMultiplier->GetValue(Particle.RelativeTime, Owner);
		Particle.Size *= SizeScale;
	}

	void ParticleModuleSizeByLife::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		if ( !Owner || !Owner->HasActiveParticles() || !Owner->ParticleData || !Owner->ParticleIndices )
		{
			return;
		}

		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride));
		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

		BEGIN_UPDATE_LOOP
		{
			Vector SizeScale = LifeMultiplier->GetValue(Particle.RelativeTime, Owner);
			ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride));
			ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
			Particle.Size *= SizeScale;
		}
		END_UPDATE_LOOP;
	}

// -------------------------------------------------------------------------------------

#if WITH_EDITOR
	bool ParticleModuleSize::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= StartSize->Draw(StartSize, "Start Size");
		return bDirty;
	}

	bool ParticleModuleSizeScale::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= SizeScale->Draw(SizeScale, "Start Size");
		return bDirty;
	}

	bool ParticleModuleSizeByLife::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= LifeMultiplier->Draw(LifeMultiplier, "Start Size");
		return bDirty;
	}
#endif



}  // namespace Drn