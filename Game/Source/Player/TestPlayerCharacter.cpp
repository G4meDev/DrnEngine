#include "GamePCH.h"
#include "TestPlayerCharacter.h"

#include "Editor/Misc/EditorMisc.h"
#include "Runtime/Components/SpringArmComponent.h"

#if WITH_EDITOR
#include <Imgui.h>
#endif

namespace Drn
{
	TestPlayerCharacter::TestPlayerCharacter()
		: Character()
	{
		m_SpringArm = std::make_shared<class SpringArmComponent>();
		GetRoot()->AttachSceneComponent(m_SpringArm.get());
		m_SpringArm->SetComponentLabel("SpringArm");
	}

	TestPlayerCharacter::~TestPlayerCharacter()
	{
		
	}

	void TestPlayerCharacter::Serialize( Archive& Ar )
	{
		Character::Serialize(Ar);

		if (Ar.IsLoading())
		{
		}

		else
		{
		}
	}

#if WITH_EDITOR
	bool TestPlayerCharacter::DrawDetailPanel()
	{
		bool Dirty = Character::DrawDetailPanel();

		return Dirty;
	}

	void TestPlayerCharacter::DrawEditorDefault()
	{
		Character::DrawEditorDefault();

	}

	void TestPlayerCharacter::DrawEditorSelected()
	{
		Character::DrawEditorSelected();

	}

#endif
}