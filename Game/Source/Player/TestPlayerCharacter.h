#pragma once

#include "ForwardTypes.h"
#include "Runtime/GameFramework/Character.h"
#include "GameForwardTypes.h"

namespace Drn
{
	class TestPlayerCharacter : public Character
	{
	public:
		TestPlayerCharacter();
		virtual ~TestPlayerCharacter();

		virtual void Serialize( Archive& Ar ) override;

		virtual EActorType GetActorType() override { return static_cast<EActorType>(EGameActorType::TestPlayerCharacter); }
		inline static EActorType GetActorTypeStatic() { return static_cast<EActorType>(EGameActorType::TestPlayerCharacter); };

#if WITH_EDITOR
		virtual bool DrawDetailPanel() override;
		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
#endif

	protected:

		std::shared_ptr<class SpringArmComponent> m_SpringArm;

	};
}