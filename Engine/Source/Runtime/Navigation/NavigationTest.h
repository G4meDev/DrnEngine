#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class NavigationTest : public Actor
	{
	public:
		NavigationTest();
		virtual ~NavigationTest();

		virtual void Serialize(Archive& Ar) override;

		virtual void Tick(float DeltaTime) override;
		inline virtual EActorType GetActorType() override { return EActorType::NavigationTest; }

		std::unique_ptr<SceneComponent> Root;
		std::unique_ptr<StaticMeshComponent> NavTestStart;
		std::unique_ptr<StaticMeshComponent> NavTestEnd;

		class ThirdPersonCharacter* TestCharacter = nullptr;

#if WITH_EDITOR
		bool DrawDetailPanel() override;

		void Randomize();
#endif
	};
}