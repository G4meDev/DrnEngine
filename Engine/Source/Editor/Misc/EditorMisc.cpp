#include "DrnPCH.h"
#include "EditorMisc.h"

#if WITH_EDITOR

EditorMisc* EditorMisc::m_SingletonInstance = nullptr;

EditorMisc* EditorMisc::Get()
{
	if ( !m_SingletonInstance )
	{
		m_SingletonInstance = new EditorMisc();
	}
	return m_SingletonInstance;
}

EditorLevelSpawnable::EditorLevelSpawnable( const char* InName, std::function<Drn::Actor*( Drn::World* )> InSpawnFunc )
	: Name( InName )
	, SpawnFunc( InSpawnFunc )
{
	EditorMisc::Get()->EditorLevelSpawnables.push_back( std::move(*this) );
}

#endif