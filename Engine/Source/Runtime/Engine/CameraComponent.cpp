#include "DrnPCH.h"
#include "CameraComponent.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	CameraComponent::CameraComponent()
		:SceneComponent()
		, m_Perspective(true)
		, m_FOV(45.0f)
		, m_OrthoWidth(1.0f)
		, m_ClipMin(0.1f)
		, m_ClipMax(10000.0f)
	{
	}

	CameraComponent::~CameraComponent()
	{
		
	}

	void CameraComponent::Tick( float DeltaTime )
	{
		SceneComponent::Tick(DeltaTime);


	}

	void CameraComponent::Serialize( Archive& Ar )
	{
		SceneComponent::Serialize(Ar);

		if ( Ar.IsLoading() )
		{
			Ar >> m_Perspective;
			Ar >> m_FOV;
			Ar >> m_OrthoWidth;
			Ar >> m_ClipMin;
			Ar >> m_ClipMax;
		}

		else
		{
			Ar << m_Perspective;
			Ar << m_FOV;
			Ar << m_OrthoWidth;
			Ar << m_ClipMin;
			Ar << m_ClipMax;
		}
	}

	void CameraComponent::GetCameraView( ViewInfo& Info )
	{
		Info.Location = GetWorldLocation();
		Info.Rotation = GetWorldRotation();

		Info.ProjectionMode = ECameraProjectionMode::Perspective;
		Info.FOV = m_FOV;
		Info.NearClipPlane = m_ClipMin;
		Info.FarClipPlane = m_ClipMax;

		Info.OrthoWidth = 10.0f;
		Info.AspectRatio = 1.0f;
	}

#if WITH_EDITOR

	void CameraComponent::DrawDetailPanel( float DeltaTime )
	{
		SceneComponent::DrawDetailPanel(DeltaTime);

		ImGui::Checkbox( "Perspective", &m_Perspective);
		if (m_Perspective)
		{
			ImGui::DragFloat("FOV", &m_FOV, 0.5f, 1.0f, 90.0f, "%.1f");
		}
		else
		{
			ImGui::DragFloat("OrthoWidth", &m_OrthoWidth, 0.5f, 1.0f, 90.0f, "%.1f");
		}
		
		ImGui::DragFloat("ClipMin", &m_ClipMin, 0.01f, 0.001f, 10.0f, "%.2f");
		ImGui::DragFloat("ClipMax", &m_ClipMax, 1.0f, 1.0f, 10000.0f, "%.1f");
	}

	void CameraComponent::DrawFrustum()
	{
		if (GetWorld())
		{
			ViewInfo VInfo;
			GetCameraView(VInfo);

			Matrix ViewProjectionMatrix = VInfo.CalculateViewMatrix() * VInfo.CalculateProjectionMatrix();
			GetWorld()->DrawDebugFrustum(ViewProjectionMatrix.Inverse(), Color::White, 0, 0);
		}
	}

	void CameraComponent::DrawEditorDefault()
	{
		
	}

	void CameraComponent::DrawEditorSelected()
	{
		DrawFrustum();
	}
#endif
}