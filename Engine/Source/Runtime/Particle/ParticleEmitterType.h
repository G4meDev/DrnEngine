#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class ParticleEmitterType : public RefCountedObject, public Serializable
	{
	public:
		ParticleEmitterType(){}

		virtual EEmitterType GetType() const = 0;

		virtual void Serialize(Archive& Ar) override
		{
			if (!Ar.IsLoading())
			{
				Ar << (uint8)GetType();
			}
		}

		static ParticleEmitterType* Create(EEmitterType Type);
		static ParticleEmitterType* Create(Archive& Ar);

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleEmitterType>& Ptr);
#endif
	};

	class ParticleEmitterCpuSpriteType : public ParticleEmitterType
	{
	public:
		ParticleEmitterCpuSpriteType() : ParticleEmitterType(){}
		virtual EEmitterType GetType() const override { return EEmitterType::Sprite_Cpu; };

	};

	class ParticleEmitterGpuSpriteType : public ParticleEmitterType
	{
	public:
		ParticleEmitterGpuSpriteType() : ParticleEmitterType(){}
		virtual EEmitterType GetType() const override { return EEmitterType::Sprite_Gpu; };

	};

	class ParticleEmitterMeshType : public ParticleEmitterType
	{
	public:
		ParticleEmitterMeshType();
		virtual EEmitterType GetType() const override { return EEmitterType::Mesh; };

		virtual void Serialize(Archive& Ar) override;

		AssetHandle<StaticMesh> Mesh;
		std::vector<MaterialSlot> Materials;

#if WITH_EDITOR
		virtual bool Draw(TRefCountPtr<ParticleEmitterType>& Ptr) override;
#endif
	};
}