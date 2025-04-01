#include "DrnPCH.h"
#include "ViewportCameraInputHandler.h"

#if WITH_EDITOR

#include <imgui.h>

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
				m_MouseDelta = m_MouseDelta * DeltaTime;
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

			m_Displacement = XMVectorSet( RightDis * DeltaTime, UpDis * DeltaTime, ForwardDis * DeltaTime, 0 );

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