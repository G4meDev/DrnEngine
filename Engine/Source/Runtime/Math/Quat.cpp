#include "DrnPCH.h"
#include "Quat.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Quat Quat::Identity = Quat();

#if WITH_EDITOR
	void Quat::Draw(const std::string& id)
	{
		float Value[4];
		Value[0] = GetX();
		Value[1] = GetY();
		Value[2] = GetZ();
		Value[3] = GetW();

		ImGui::PushID(id.c_str());
		if ( ImGui::DragFloat4( "", Value, 0.3f, 0, 0, "%.3f" ) )
		{
			m_Vector = XMQuaternionNormalize( XMVectorSet( Value[0], Value[1], Value[2], Value[3] ) );
		}
		ImGui::PopID();
	}
#endif
}