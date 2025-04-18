#include "DrnPCH.h"
#include "Vector.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Vector Vector::ZeroVector	= Vector(0);
	Vector Vector::OneVector	= Vector(1);

#if WITH_EDITOR
	void Vector::Draw(const std::string& id)
	{
		XMVECTOR Vec = XMLoadFloat3(&m_Vector);

		float Value[3];
		Value[0] = XMVectorGetX(Vec);
		Value[1] = XMVectorGetY(Vec);
		Value[2] = XMVectorGetZ(Vec);

		ImGui::PushID( id.c_str() );
		if (ImGui::DragFloat3("", Value, 0.3f, 0, 0, "%.3f"))
		{
			XMStoreFloat3( &m_Vector, XMVectorSet( Value[0], Value[1], Value[2], 0 ) );
		}
		ImGui::PopID();
	}
#endif
}