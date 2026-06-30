#include "DrnPCH.h"
#include "ParticleSystem.h"

#include "Runtime/Particle/ParticleEmitter.h"
#include "Editor/AssetPreview/AssetPreviewParticleSystemGuiLayer.h"

namespace Drn
{
	ParticleSystem::ParticleSystem( const std::string& Path )
		: Asset( Path )
	{
		Load();
	}

#if WITH_EDITOR
	ParticleSystem::ParticleSystem( const std::string& InPath, const std::string& InSourcePath )
		: Asset( InPath )
	{
		Save();
	}
#endif

	ParticleSystem::~ParticleSystem()
	{
		
	}

	void ParticleSystem::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			uint8 EmitterCount = 0;
			Ar >> EmitterCount;
			Emitters.resize(EmitterCount);
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterCount; EmitterIndex++)
			{
				Emitters[EmitterIndex] = new ParticleEmitter();
				Emitters[EmitterIndex]->Serialize(Ar);
			}
		}

		else
		{
			const uint8 EmitterCount = std::min(Emitters.size(), (size_t)UINT8_MAX);
			Ar << EmitterCount;
			for (int32 EmitterIndex = 0; EmitterIndex < EmitterCount; EmitterIndex++)
			{
				Emitters[EmitterIndex]->Serialize(Ar);
			}
		}
	}

	EAssetType ParticleSystem::GetAssetType()
	{
		return EAssetType::ParticleSystem;
	}

#if WITH_EDITOR

	void ParticleSystem::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewParticleSystemGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void ParticleSystem::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

#endif

}