#include "DrnPCH.h"
#include "Level.h"
#include "Runtime/Engine/DirectionalLightActor.h"
#include "Runtime/Engine/SkyLightActor.h"
#include "Runtime/Engine/PostProcessVolume.h"

namespace Drn
{
	Level::Level( const std::string& Path )
		: Asset(Path)
	{

	}

#if WITH_EDITOR
	Level::Level( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
	{
		Save();
		std::cout << InPath;
	}
#endif

	Level::~Level()
	{
		
	}

	void Level::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{

		}

#if WITH_EDITOR
		else
		{
			// default actor count
			Ar << (uint32)0;
		}
#endif
	}

	EAssetType Level::GetAssetType()
	{
		return EAssetType::Level;
	}

#if WITH_EDITOR

	void Level::OpenAssetPreview()
	{
		WorldManager::Get()->LoadLevel(m_Path);
	}

	void Level::CloseAssetPreview()
	{
		
	}

	void Level::SaveFromWorld( World* InWorld )
	{
		FileArchive Ar(Path::ConvertProjectPath(m_Path), false);
		Asset::Serialize(Ar);

		uint32 ActorCount = InWorld->GetNonTransientActorCount();
		Ar << ActorCount;

		for (Actor* actor : InWorld->m_Actors)
		{
			if (actor->IsTransient())
			{
				continue;
			}

			Ar << static_cast<uint16>(actor->GetActorType());
			actor->Serialize(Ar);
		}
	}

#endif

	void Level::LoadToWorld( World* InWorld )
	{
		std::string LevelLabel = Path::ConvertShortPath(m_Path);
		LevelLabel = Path::RemoveFileExtension(LevelLabel);
		
		InWorld->m_LevelPath = m_Path;
		
#if WITH_EDITOR
		InWorld->SetLabel(LevelLabel);
#endif

		FileArchive Ar(Path::ConvertProjectPath(m_Path));
		Asset::Serialize( Ar );

		uint32 ActorCount;
		Ar >> ActorCount;
		
		uint16 ActorTypeByte;
		EActorType ActorType;

		for (int32 i = 0; i < ActorCount; i++)
		{
			Ar >> ActorTypeByte;
			ActorType = static_cast<EActorType>(ActorTypeByte);

			if (ActorType == EActorType::StaticMeshActor) { SerializeActorToWorld<StaticMeshActor>(InWorld, Ar); }
			else if ( ActorType == EActorType::PointLight ) { SerializeActorToWorld<PointLightActor>(InWorld, Ar); }
			else if ( ActorType == EActorType::SpotLight ){ SerializeActorToWorld<SpotLightActor>(InWorld, Ar); }
			else if ( ActorType == EActorType::DirectionalLight ) { SerializeActorToWorld<DirectionalLightActor>(InWorld, Ar); }
			else if ( ActorType == EActorType::SkyLight ) { SerializeActorToWorld<SkyLightActor>(InWorld, Ar); }
			else if ( ActorType == EActorType::PostProcessVolume ) { SerializeActorToWorld<PostProcessVolume>(InWorld, Ar); }
			else if ( ActorType == EActorType::Pawn ) { SerializeActorToWorld<Pawn>(InWorld, Ar); }
			else if ( ActorType == EActorType::Character ) { SerializeActorToWorld<Character>(InWorld, Ar); }

		}
	
	}


}