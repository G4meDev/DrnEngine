#include "DrnPCH.h"
#include "ViewportCameraInputHandler.h"

#if WITH_EDITOR

#include <imgui.h>

#define CAMERA_MOVE_SPEED_SCALAR 1400.0f;

namespace Drn
{
	ViewportCameraInputHandler::ViewportCameraInputHandler()
		: m_CapturingMouse(false)
	{
		
	}

	bool ViewportCameraInputHandler::Tick( float DeltaTime )
	{
		bool bHovering  = ImGui::IsWindowHovered();
		bool bMouseDown = ImGui::IsKeyDown( ImGuiKey_MouseRight );

		m_MouseDelta = IntPoint(0);
		m_Displacement = XMVectorSet(0, 0, 0, 0);

		if ( bHovering && bMouseDown )
		{
			IntPoint CurerentMousePos(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

			if (m_CapturingMouse)
			{
				m_MouseDelta = CurerentMousePos - m_LastMousePos;
				m_MouseDelta.X = m_MouseDelta.X;
				m_MouseDelta.Y = m_MouseDelta.Y;
			}

			else
			{
				m_CapturingMouse = true;
			}

			m_LastMousePos = CurerentMousePos;

			bool wDown = ImGui::IsKeyDown( ImGuiKey::ImGuiKey_W );
			bool aDown = ImGui::IsKeyDown( ImGuiKey::ImGuiKey_A );
			bool sDown = ImGui::IsKeyDown( ImGuiKey::ImGuiKey_S );
			bool dDown = ImGui::IsKeyDown( ImGuiKey::ImGuiKey_D );
			bool eDown = ImGui::IsKeyDown( ImGuiKey::ImGuiKey_E );
			bool qDown = ImGui::IsKeyDown( ImGuiKey::ImGuiKey_Q );

			float ForwardDis = wDown - sDown;
			float RightDis   = dDown - aDown;
			float UpDis      = eDown - qDown;

			const float CameraMoveScalar = DeltaTime * CAMERA_MOVE_SPEED_SCALAR;
			m_Displacement = XMVectorSet( RightDis * CameraMoveScalar, UpDis * CameraMoveScalar, ForwardDis * CameraMoveScalar, 0 );

			return true;
		}
		else
		{
			m_CapturingMouse = false;
			return false;
		}
	}
}

#endif