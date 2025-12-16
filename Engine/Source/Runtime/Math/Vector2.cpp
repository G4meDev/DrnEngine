#include "DrnPCH.h"
#include "Vector2.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Vector Vector2::ZeroVector(0);
	Vector Vector2::OneVector(1);

#if WITH_EDITOR
	bool Vector2::Draw( const std::string& id )
	{
		ImGui::Text("Vector2");
		return false;
	}

	bool Vector2::Draw()
	{
		float Value[2] = { GetX(), GetY() };
		if (ImGui::DragFloat2("", Value, 0.3f, 0, 0, "%.3f"))
		{
			XMStoreFloat2(&m_Vector, XMVectorSet( Value[0], Value[1], 0, 0));
			return true;
		}

		return false;
	}

#endif
}