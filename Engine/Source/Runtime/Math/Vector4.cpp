#include "DrnPCH.h"
#include "Vector4.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
#if WITH_EDITOR
	bool Vector4::Draw( const std::string& id, const std::string& Label)
	{
		bool bDirty = false;

		float Value[4] = { GetX(), GetY(), GetZ(), GetW() };
		ImGui::PushID( id.c_str() );
		if (ImGui::DragFloat4(Label.c_str(), Value, 0.3f, 0, 0, "%.3f"))
		{
			XMStoreFloat4(&m_Vector, XMVectorSet( Value[0], Value[1], Value[2], Value[3] ));
			bDirty = true;
		}
		ImGui::PopID();

		return bDirty;
	}

	bool Vector4::Draw()
	{
		float Value[4] = { GetX(), GetY(), GetZ(), GetW() };
		if (ImGui::DragFloat4("", Value, 0.3f, 0, 0, "%.3f"))
		{
			XMStoreFloat4(&m_Vector, XMVectorSet( Value[0], Value[1], Value[2], Value[3] ));
			return true;
		}

		return false;
	}
#endif
}