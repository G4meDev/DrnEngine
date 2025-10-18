#include "DrnPCH.h"
#include "EditorMisc.h"

#include "Runtime/Engine/SpotLightActor.h"
#include "Runtime/Engine/PostProcessVolume.h"
#include "Runtime/Engine/DecalActor.h"

#if WITH_EDITOR

namespace Drn
{
	EditorMisc* EditorMisc::m_SingletonInstance = nullptr;

	EditorMisc::EditorMisc()
	{
		EditorLevelSpawnablesCategories.insert("All");
	}

	void EditorMisc::Register()
	{
		REGISTER_LEVEL_SPAWNABLE_CLASS( PointLightActor			, Light );
		REGISTER_LEVEL_SPAWNABLE_CLASS( SpotLightActor			, Light );
		REGISTER_LEVEL_SPAWNABLE_CLASS( SkyLightActor			, Light );
		REGISTER_LEVEL_SPAWNABLE_CLASS( DirectionalLightActor	, Light );
		REGISTER_LEVEL_SPAWNABLE_CLASS( PostProcessVolume		, Volume );
		REGISTER_LEVEL_SPAWNABLE_CLASS( DecalActor				, Volume );
		REGISTER_LEVEL_SPAWNABLE_CLASS( Pawn					, Player );
		REGISTER_LEVEL_SPAWNABLE_CLASS( Character				, Player );
	}

	void EditorMisc::RegisterLevelSpawnableClass( const char* Name, const char* Category, std::function<Actor*(World*)>&& SpawnFunc)
	{
		EditorLevelSpawnables.emplace_back(Name, Category, std::move(SpawnFunc));
		EditorMisc::Get()->EditorLevelSpawnablesCategories.insert(Category);
	}

	EditorMisc* EditorMisc::Get()
	{
		if ( !m_SingletonInstance )
		{
			m_SingletonInstance = new EditorMisc();
		}
		return m_SingletonInstance;
	}

}

#endif