#pragma once

#include "ForwardTypes.h"

#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/World.h"

#if WITH_EDITOR

struct EditorLevelSpawnable
{
	EditorLevelSpawnable(const char* InName, const char* InCategory, std::function<Drn::Actor*(Drn::World*)> InSpawnFunc);

	std::string Name;
	std::string Category;
	std::function<Drn::Actor*(Drn::World*)> SpawnFunc;
};

class EditorMisc
{
public:

	EditorMisc();
	~EditorMisc() {};

	static EditorMisc* Get();

	static EditorMisc* m_SingletonInstance;
	std::vector<EditorLevelSpawnable> EditorLevelSpawnables;
	std::set<std::string> EditorLevelSpawnablesCategories;
};

#define DECLARE_LEVEL_SPAWNABLE_CLASS( name , category )												\
	EditorLevelSpawnable EditorSpawn_##name( #name , #category , []( Drn::World* InWorld ) {							\
		Drn::Actor* actor = InWorld->SpawnActor<name>(); actor->SetActorLabel(#name); return actor; });
#else
#define DECLARE_LEVEL_SPAWNABLE_CLASS( name , category);
#endif