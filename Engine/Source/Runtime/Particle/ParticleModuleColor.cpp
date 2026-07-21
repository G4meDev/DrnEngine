#include "DrnPCH.h"
#include "ParticleModuleColor.h"

namespace Drn
{
	ParticleModuleColor::ParticleModuleColor()
		: ParticleModule()
		, StartColor(new ParticleDistributionVectorConstant(Vector::OneVector))
		, StartAlpha(new ParticleDistributionFloatConstant(1.0f))
	{
		bSpawnModule = true;
	}

	void ParticleModuleColor::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartColor = ParticleDistributionVector::Create(Ar);
			StartAlpha = ParticleDistributionFloat::Create(Ar);
		}
		else
		{
			StartColor->Serialize(Ar);
			StartAlpha->Serialize(Ar);
		}
	}

	void ParticleModuleColor::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			Vector ColorVec	= StartColor->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
			float Alpha = StartAlpha->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
			Particle.Color = Vector4(ColorVec, Alpha);
			Particle.BaseColor	= Particle.Color;
		}
	}

// -----------------------------------------------------------------------------------------------------------

	ParticleModuleColorOverLife::ParticleModuleColorOverLife()
		: ParticleModule()
		, ColorOverLife(new ParticleDistributionVectorConstant(Vector::OneVector))
		, AlphaOverLife(new ParticleDistributionFloatConstant(1.0f))
	{
		bSpawnModule = true;
		bUpdateModule = true;
	}

	void ParticleModuleColorOverLife::Serialize( Archive& Ar )
	{
		ParticleModule::Serialize(Ar);

		if (Ar.IsLoading())
		{
			ColorOverLife = ParticleDistributionVector::Create(Ar);
			AlphaOverLife = ParticleDistributionFloat::Create(Ar);
		}
		else
		{
			ColorOverLife->Serialize(Ar);
			AlphaOverLife->Serialize(Ar);
		}
	}

	void ParticleModuleColorOverLife::Spawn( ParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;
		{
			Vector ColorVec	= ColorOverLife->GetValue(ParticleBase->RelativeTime, Owner, &GetRandomStream(Owner));
			float Alpha = AlphaOverLife->GetValue(ParticleBase->RelativeTime, Owner, &GetRandomStream(Owner));
			Particle.Color = Vector4(ColorVec, Alpha);
			Particle.BaseColor	= Particle.Color;
		}
	}

	void ParticleModuleColorOverLife::Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime )
	{
		if ( !Owner || !Owner->HasActiveParticles() || !Owner->ParticleData || !Owner->ParticleIndices )
		{
			return;
		}

		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride));
		ApplicationMisc::Prefetch(Owner->ParticleData, (Owner->ParticleIndices[0] * Owner->ParticleStride) + PLATFORM_CACHE_LINE_SIZE);

		BEGIN_UPDATE_LOOP
		{
			Vector ColorVec	= ColorOverLife->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner));
			float Alpha = AlphaOverLife->GetValue(Particle.RelativeTime, Owner, &GetRandomStream(Owner));
			ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride));
			ApplicationMisc::Prefetch(ParticleData, (ParticleIndices[i+1] * ParticleStride) + PLATFORM_CACHE_LINE_SIZE);
			Particle.Color = Vector4(ColorVec, Alpha);
		}
		END_UPDATE_LOOP;
	}

// -----------------------------------------------------------------------------------------------------------

#if WITH_EDITOR
	bool ParticleModuleColor::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= StartColor->Draw(StartColor, "Start Color");
		bDirty |= StartAlpha->Draw(StartAlpha, "Start Alpha");
		return bDirty;
	}

	bool ParticleModuleColorOverLife::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModule::Draw(Owner);
		bDirty |= ColorOverLife->Draw(ColorOverLife, "Color Over Life");
		bDirty |= AlphaOverLife->Draw(AlphaOverLife, "Alpha Over Life");
		return bDirty;
	}
#endif

}  // namespace Drn