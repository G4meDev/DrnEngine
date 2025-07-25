#include "DrnPCH.h"
#include "Level.h"

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
		Archive Ar(Path::ConvertProjectPath(m_Path), false);
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

		Archive Ar(Path::ConvertProjectPath(m_Path));
		Asset::Serialize( Ar );

		uint32 ActorCount;
		Ar >> ActorCount;
		
		uint16 ActorTypeByte;
		EActorType ActorType;

		for (int32 i = 0; i < ActorCount; i++)
		{
			Ar >> ActorTypeByte;
			ActorType = static_cast<EActorType>(ActorTypeByte);

			if (ActorType == EActorType::StaticMeshActor)
			{
				StaticMeshActor* NewActor = InWorld->SpawnActor<StaticMeshActor>();
				NewActor->Serialize(Ar);
			}

			else if ( ActorType == EActorType::PointLight )
			{
				PointLightActor* NewPointLightActor = InWorld->SpawnActor<PointLightActor>();
				NewPointLightActor->Serialize(Ar);
			}
		}
	
	}


}