#include "DrnPCH.h"
#include "ParticleModuleLocation.h"

namespace Drn
{
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

	void ParticleModuleLocationPrimitiveSphere::Spawn( ParticleEmitterInstance* Owner, float SpawnTime, BaseParticle* ParticleBase )
	{
		SPAWN_INIT;

		// Determine the start location for the sphere
		//Vector vStartLoc = StartLocation.GetValue(Owner->EmitterTime, Owner->Component, 0, InRandomStream);
		Vector vStartLoc = StartLocation;

		Vector vUnitDir;
		DetermineUnitDirection(Owner, vUnitDir, &GetRandomStream(Owner));

		Vector vNormalizedDir = vUnitDir;
		vNormalizedDir = vNormalizedDir.GetUnsafeNormal();

		if (SurfaceOnly)
		{
			vUnitDir = vUnitDir.GetUnsafeNormal();
		}

		//float	fStartRadius	= StartRadius.GetValue(Owner->EmitterTime, Owner->Component, InRandomStream);
		float	fStartRadius	= StartRadius;
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

		//if (Velocity)
		//{
		//	FVector vVelocity		 = (vOffset - vStartLoc) * VelocityScale.GetValue(Owner->EmitterTime, Owner->Component, InRandomStream);
		//	vVelocity = Owner->EmitterToSimulation.TransformVector(vVelocity);
		//	Particle.Velocity		+= vVelocity;
		//	Particle.BaseVelocity	+= vVelocity;
		//}
	}

        }