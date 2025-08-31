#pragma once

#include "ForwardTypes.h"

#if WITH_EDITOR

#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/World.h"

namespace Drn
{
	struct EditorLevelSpawnable
	{
		EditorLevelSpawnable(const char* InName, const char* InCategory, std::function<Actor*(World*)>&& InSpawnFunc)
			: Name( InName )
			, Category( InCategory )
			, SpawnFunc( InSpawnFunc )
		{}

		std::string Name;
		std::string Category;
		std::function<Actor*(World*)> SpawnFunc;
	};

	class EditorMisc
	{
	public:

		EditorMisc();
		~EditorMisc() {};


		void Register();
		void RegisterLevelSpawnableClass(const char* Name, const char* Category, std::function<Actor*(World*)>&& SpawnFunc);

		static EditorMisc* Get();

		static EditorMisc* m_SingletonInstance;
		std::vector<EditorLevelSpawnable> EditorLevelSpawnables;
		std::set<std::string> EditorLevelSpawnablesCategories;
	};

#define REGISTER_LEVEL_SPAWNABLE_CLASS( name , category )											\
	EditorMisc::Get()->RegisterLevelSpawnableClass( #name, #category, []( World* InWorld ) {		\
		Actor* actor = InWorld->SpawnActor<name>(); actor->SetActorLabel(#name); return actor; });
}

#else
#define REGISTER_LEVEL_SPAWNABLE_CLASS( name, category )
#endif