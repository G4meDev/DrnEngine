#pragma once

#include "ForwardTypes.h"

#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/World.h"

#if WITH_EDITOR

struct EditorLevelSpawnable
{
	EditorLevelSpawnable(const char* InName, std::function<Drn::Actor*(Drn::World*)> InSpawnFunc);

	std::string Name;
	std::function<Drn::Actor*(Drn::World*)> SpawnFunc;

};

class EditorMisc
{
public:

	EditorMisc() {};
	~EditorMisc() {};

	inline static EditorMisc* Get();

	static EditorMisc* m_SingletonInstance;
	std::vector<EditorLevelSpawnable> EditorLevelSpawnables;
};

#define DECLARE_LEVEL_SPAWNABLE_CLASS( name )									\
	EditorLevelSpawnable EditorSpawn_##name(#name, []( Drn::World* InWorld ) {	\
		Drn::Actor* actor = InWorld->SpawnActor<name>(); return actor; });
#else
#define DECLARE_LEVEL_SPAWNABLE_CLASS( name );
#endif