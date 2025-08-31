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
		m_SpringArm = std::make_shared<SpringArmComponent>();
		GetRoot()->AttachSceneComponent(m_SpringArm.get());
		m_SpringArm->SetComponentLabel("SpringArm");

		m_Camera = std::make_shared<CameraComponent>();
		m_SpringArm->AttachSceneComponent(m_Camera.get());
		m_Camera->SetComponentLabel("Camera");
	}

	TestPlayerCharacter::~TestPlayerCharacter()
	{
		
	}

	void TestPlayerCharacter::Serialize( Archive& Ar )
	{
		Character::Serialize(Ar);

		m_SpringArm->Serialize(Ar);
		m_Camera->Serialize(Ar);
	}

	void TestPlayerCharacter::CalcCamera( struct ViewInfo& OutResult )
	{
		m_Camera->GetCameraView(OutResult);
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