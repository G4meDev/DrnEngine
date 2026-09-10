#include "DrnPCH.h"
#include "ThirdPersonCharacter.h"

#include "Editor/Misc/EditorMisc.h"
#include "Runtime/Components/InputComponent.h"
#include "Runtime/Components/SpringArmComponent.h"

#if WITH_EDITOR
#include <Imgui.h>
#endif

namespace Drn
{
	ThirdPersonCharacter::ThirdPersonCharacter()
		: Character()
	{
		CharacterMesh = std::make_shared<SkeletalMeshComponent>();
		GetRoot()->AttachSceneComponent(CharacterMesh.get());
		CharacterMesh->SetComponentLabel("Character Mesh");

		m_SpringArm = std::make_shared<SpringArmComponent>();
		GetRoot()->AttachSceneComponent(m_SpringArm.get());
		m_SpringArm->SetComponentLabel("SpringArm");
		m_SpringArm->SetRelativeLocation(Vector(0.0f, 3.0f, 0.0f));

		m_Camera = std::make_shared<CameraComponent>();
		m_SpringArm->AttachSceneComponent(m_Camera.get());
		m_Camera->SetComponentLabel("Camera");


		AssetHandle<SkeletalMesh> MeshAsset("Engine\\Content\\Template\\ThirdPersonCharacter\\Character\\SK_ThirdPersonCharacter.drn");
		MeshAsset.Load();
		CharacterMesh->SetMesh(MeshAsset);
	}

	ThirdPersonCharacter::~ThirdPersonCharacter()
	{
		
	}

	void ThirdPersonCharacter::Serialize( Archive& Ar )
	{
		Character::Serialize(Ar);

		m_SpringArm->Serialize(Ar);
		m_Camera->Serialize(Ar);
		CharacterMesh->Serialize(Ar);
	}

	void ThirdPersonCharacter::Tick( float DeltaTime )
	{
		Character::Tick(DeltaTime);

		Vector ForwardVector = m_SpringArm->GetForwardVector() * Vector(1, 0, 1);
		ForwardVector = ForwardVector.GetSafeNormal();

		Vector RightVector = Vector::CrossProduct( Vector::UpVector, ForwardVector );

		m_MovementInput = ForwardVector * m_ForwardInput + RightVector * m_RightInput;
		m_MovementInput = m_MovementInput * DeltaTime * (m_Running ? m_RunSpeed : m_WalkSpeed);

		m_ForwardInput = m_RightInput = 0;

		m_SpringArm->SetWorldRotation(CameraRotation.Quaternion());
	}

	void ThirdPersonCharacter::CalcCamera( struct ViewInfo& OutResult )
	{
		m_Camera->GetCameraView(OutResult);
	}

	void ThirdPersonCharacter::SetupPlayerInputComponent( class InputComponent* PlayerInputComponent )
	{
		PlayerInputComponent->AddAxis(1, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnMoveForward);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyW, 1);
		PlayerInputComponent->AddAxisMapping(1, gainput::KeyS, -1);

		PlayerInputComponent->AddAxis(2, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnMoveRight);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyD, 1);
		PlayerInputComponent->AddAxisMapping(2, gainput::KeyA, -1);

		PlayerInputComponent->AddAnalog(3, this, &ThirdPersonCharacter::OnLookRight);
		PlayerInputComponent->AddAnalogMapping(3, gainput::MouseAxisX, -1);
		
		PlayerInputComponent->AddAnalog(4, this, &ThirdPersonCharacter::OnLookUp);
		PlayerInputComponent->AddAnalogMapping(4, gainput::MouseAxisY, 1);

		PlayerInputComponent->AddKey(5, this, &ThirdPersonCharacter::OnBeginRun, &ThirdPersonCharacter::OnEndRun);
		PlayerInputComponent->AddKeyMapping(5, gainput::KeyShiftL);

		PlayerInputComponent->AddAxis(6, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnLookUp);
		PlayerInputComponent->AddAxisMapping(6, gainput::KeyUp, 1);
		PlayerInputComponent->AddAxisMapping(6, gainput::KeyDown, -1);

		PlayerInputComponent->AddAxis(7, 1.0f, 1.0f, this, &ThirdPersonCharacter::OnLookRight);
		PlayerInputComponent->AddAxisMapping(7, gainput::KeyRight, 1);
		PlayerInputComponent->AddAxisMapping(7, gainput::KeyLeft, -1);
	}

	void ThirdPersonCharacter::OnMoveForward( float Value )
	{
		m_ForwardInput = Value;
	}

	void ThirdPersonCharacter::OnMoveRight( float Value )
	{
		m_RightInput = Value;
	}

	void ThirdPersonCharacter::OnLookUp( float Value )
	{
		CameraRotation.Pitch += Time::GetApplicationDeltaTime() * -m_LookSpeed * Value;
		CameraRotation.Pitch = std::clamp(CameraRotation.Pitch, -m_CameraPitchClamp, m_CameraPitchClamp);
	}

	void ThirdPersonCharacter::OnLookRight( float Value )
	{
		CameraRotation.Yaw += Time::GetApplicationDeltaTime() * m_LookSpeed * Value;
	}

	void ThirdPersonCharacter::OnBeginRun()
	{
		m_Running = true;
	}

	void ThirdPersonCharacter::OnEndRun()
	{
		m_Running = false;
	}

#if WITH_EDITOR
	bool ThirdPersonCharacter::DrawDetailPanel()
	{
		bool Dirty = Character::DrawDetailPanel();

		return Dirty;
	}

	void ThirdPersonCharacter::DrawEditorDefault()
	{
		Character::DrawEditorDefault();

	}

	void ThirdPersonCharacter::DrawEditorSelected()
	{
		Character::DrawEditorSelected();

	}

#endif
}