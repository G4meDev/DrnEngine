#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleDistributionFloat;

	class ParticleModuleKillHeight : public ParticleModule
	{
	public:
		ParticleModuleKillHeight();
	
		TRefCountPtr<ParticleDistributionFloat> Height;
		bool bAbsolute;
		bool bFloor;
		bool bApplyPSysScale;
	
		virtual EParticleModule GetModuleType() const override { return EParticleModule::KillHeight; }
		virtual void Serialize( Archive& Ar ) override;

		virtual void Update( ParticleEmitterInstance* Owner, int32 Offset, float DeltaTime ) override;	
	
	#if WITH_EDITOR
		virtual bool Draw( ParticleEmitter* Owner ) override;
	#endif
	};
}