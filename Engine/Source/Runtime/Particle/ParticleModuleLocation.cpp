#include "DrnPCH.h"
#include "ParticleModuleLocation.h"

namespace Drn
{
	ParticleModuleLocationPrimitiveBase::ParticleModuleLocationPrimitiveBase()
		: ParticleModuleLocationBase()
		, Positive_X(1)
		, Positive_Y(1)
		, Positive_Z(1)
		, Negative_X(1)
		, Negative_Y(1)
		, Negative_Z(1)
		, SurfaceOnly(0)
		, Velocity(0)
		, VelocityScale(new ParticleDistributionFloatConstant(1.0f))
		, StartLocation(new ParticleDistributionVectorConstant(Vector::ZeroVector))
	{}

	void ParticleModuleLocationPrimitiveBase::Serialize( Archive& Ar )
	{
		ParticleModuleLocationBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Positive_X;
			Ar >> Positive_Y;
			Ar >> Positive_Z;
			Ar >> Negative_X;
			Ar >> Negative_Y;
			Ar >> Negative_Z;
			Ar >> SurfaceOnly;
			Ar >> Velocity;
			VelocityScale = ParticleDistributionFloat::Create(Ar);
			StartLocation = ParticleDistributionVector::Create(Ar);
		}

		else
		{
			Ar << Positive_X;
			Ar << Positive_Y;
			Ar << Positive_Z;
			Ar << Negative_X;
			Ar << Negative_Y;
			Ar << Negative_Z;
			Ar << SurfaceOnly;
			Ar << Velocity;
			VelocityScale->Serialize(Ar);
			StartLocation->Serialize(Ar);
		}
	}

	void ParticleModuleLocationPrimitiveBase::DetermineUnitDirection( ParticleEmitterInstance* Owner, Vector& vUnitDir, RandomStream* InRandomStream )
	{
		Vector vRand = Vector(InRandomStream->GetFraction(), InRandomStream->GetFraction(), InRandomStream->GetFraction());

		// Set the unit dir
		if (Positive_X && Negative_X)
		{
			vUnitDir.SetX(vRand.GetX() * 2 - 1);
		}
		else if (Positive_X)
		{
			vUnitDir.SetX(vRand.GetX());
		}
		else if (Negative_X)
		{
			vUnitDir.SetX(-vRand.GetX());
		}
		else
		{
			vUnitDir.SetX(0.0f);
		}

		if (Positive_Y && Negative_Y)
		{
			vUnitDir.SetY(vRand.GetY() * 2 - 1);
		}
		else if (Positive_Y)
		{
			vUnitDir.SetY(vRand.GetY());
		}
		else if (Negative_Y)
		{
			vUnitDir.SetY(-vRand.GetY());
		}
		else
		{
			vUnitDir.SetY(0.0f);
		}

		if (Positive_Z && Negative_Z)
		{
			vUnitDir.SetZ(vRand.GetZ() * 2 - 1);
		}
		else if (Positive_Z)
		{
			vUnitDir.SetZ(vRand.GetZ());
		}
		else if (Negative_Z)
		{
			vUnitDir.SetZ(-vRand.GetZ());
		}
		else
		{
			vUnitDir.SetZ(0.0f);
		}
	}

	ParticleModuleLocationPrimitiveSphere::ParticleModuleLocationPrimitiveSphere()
		: ParticleModuleLocationPrimitiveBase()
		, StartRadius(new ParticleDistributionFloatConstant(5.0f))
	{}

	void ParticleModuleLocationPrimitiveSphere::Spawn( ParticleEmitterInstance* Owner, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;

		Vector vStartLoc = StartLocation->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));

		Vector vUnitDir;
		DetermineUnitDirection(Owner, vUnitDir, &GetRandomStream(Owner));

		Vector vNormalizedDir = vUnitDir;
		vNormalizedDir = vNormalizedDir.GetUnsafeNormal();

		if (SurfaceOnly)
		{
			vUnitDir = vUnitDir.GetUnsafeNormal();
		}

		float	fStartRadius	= StartRadius->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
		Vector	vStartRadius	= Vector(fStartRadius);
		Vector	vOffset			= vUnitDir * vStartRadius;

		Vector	vMax;

		vMax.SetX( Math::Abs(vNormalizedDir.GetX()) * fStartRadius );
		vMax.SetY( Math::Abs(vNormalizedDir.GetY()) * fStartRadius );
		vMax.SetZ( Math::Abs(vNormalizedDir.GetZ()) * fStartRadius );

		if (Positive_X || Negative_X)
		{
			vOffset.SetX(std::clamp<float>(vOffset.GetX(), -vMax.GetX(), vMax.GetX()));
		}
		else
		{
			vOffset.SetX(0.0f);
		}
		if (Positive_Y || Negative_Y)
		{
			vOffset.SetY(std::clamp<float>(vOffset.GetY(), -vMax.GetY(), vMax.GetY()));
		}
		else
		{
			vOffset.SetY(0.0f);
		}
		if (Positive_Z || Negative_Z)
		{
			vOffset.SetZ(std::clamp<float>(vOffset.GetZ(), -vMax.GetZ(), vMax.GetZ()));
		}
		else
		{
			vOffset.SetZ(0.0f);
		}

		vOffset = vOffset + vStartLoc;
		Particle.Location = Particle.Location + Owner->EmitterToSimulation.TransformVector(vOffset);

		if (Velocity)
		{
			Vector vVelocity		= (vOffset - vStartLoc) * VelocityScale->GetValue(Owner->EmitterTime, Owner, &GetRandomStream(Owner));
			vVelocity				= Owner->EmitterToSimulation.TransformVector(vVelocity);
			Particle.Velocity		= Particle.Velocity + vVelocity;
			Particle.BaseVelocity	= Particle.BaseVelocity + vVelocity;
		}
	}

	void ParticleModuleLocationPrimitiveSphere::Serialize( Archive& Ar )
	{
		ParticleModuleLocationPrimitiveBase::Serialize(Ar);

		if (Ar.IsLoading())
		{
			StartRadius = ParticleDistributionFloat::Create(Ar);
		}

		else
		{
			StartRadius->Serialize(Ar);
		}
	}

#if WITH_EDITOR
	bool ParticleModuleLocationPrimitiveBase::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleLocationBase::Draw(Owner);

		bDirty |= ImGui::Checkbox("Positive X",			&Positive_X);
		bDirty |= ImGui::Checkbox("Positive Y",			&Positive_Y);
		bDirty |= ImGui::Checkbox("Positive Z",			&Positive_Z);
		bDirty |= ImGui::Checkbox("Negative X",			&Negative_X);
		bDirty |= ImGui::Checkbox("Negative Y",			&Negative_Y);
		bDirty |= ImGui::Checkbox("Negative Z",			&Negative_Z);
		bDirty |= ImGui::Checkbox("Surface Only",		&SurfaceOnly);
		bDirty |= ImGui::Checkbox("Velocity",			&Velocity);
		bDirty |= VelocityScale->Draw(VelocityScale, "Velocity Scale");
		bDirty |= StartLocation->Draw(StartLocation, "Start Location");

		return bDirty;
	}

	bool ParticleModuleLocationPrimitiveSphere::Draw( ParticleEmitter* Owner )
	{
		bool bDirty = ParticleModuleLocationPrimitiveBase::Draw(Owner);

		bDirty |= StartRadius->Draw(StartRadius, "Radius");

		return bDirty;
	}
#endif

        }  // namespace Drn