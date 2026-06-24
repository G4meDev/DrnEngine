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
			Ar >> Unused;
		}

		else
		{
			Ar << Unused;
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