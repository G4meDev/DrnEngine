#include "GamePCH.h"
#include "TestPlayerCharacter.h"

#include "Editor/Misc/EditorMisc.h"
#include "Runtime/Components/InputComponent.h"
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

	void TestPlayerCharacter::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &TestPlayerCharacter::OnMoveForward);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);

		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &TestPlayerCharacter::OnMoveRight);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);

		PlayerInputComponent->AddAnalog(3, this, &TestPlayerCharacter::OnLookRight);
		PlayerInputComponent->AddAnalogMapping(3, gainput::MouseAxisX, 1);

		PlayerInputComponent->AddAnalog(4, this, &TestPlayerCharacter::OnLookUp);
		PlayerInputComponent->AddAnalogMapping(4, gainput::MouseAxisY, 1);
	}

	void TestPlayerCharacter::OnMoveForward( float Value )
	{
		m_MovementInput.SetZ(Value * 10.0f * Time::GetApplicationDeltaTime());
	}

	void TestPlayerCharacter::OnMoveRight( float Value )
	{
		m_MovementInput.SetX(Value * 10.0f * Time::GetApplicationDeltaTime());
	}

	void TestPlayerCharacter::OnLookUp( float Value )
	{
		m_SpringArm->SetWorldRotation( Quat( m_SpringArm->GetRightVector(), Value * -0.005f) * m_SpringArm->GetWorldRotation() );
	}

	void TestPlayerCharacter::OnLookRight( float Value )
	{
		m_SpringArm->SetWorldRotation( Quat( Vector::UpVector, Value * 0.005f) * m_SpringArm->GetWorldRotation());
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