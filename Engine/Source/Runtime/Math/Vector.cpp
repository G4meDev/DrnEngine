#include "DrnPCH.h"
#include "Vector.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Vector Vector::ZeroVector = Vector( 0 );
	Vector Vector::OneVector		= Vector(1);

	Vector Vector::UpVector			= Vector(0, 1, 0);
	Vector Vector::DownVector		= Vector(0, -1, 0);

	Vector Vector::RightVector		= Vector(1, 0, 0);
	Vector Vector::LeftVector		= Vector(-1, 0, 0);

	Vector Vector::ForwardVector	= Vector(0, 0, 1);
	Vector Vector::BackwardVector	= Vector(0, 0, -1);

#if WITH_EDITOR
	bool Vector::Draw(const std::string& id, const std::string& Label)
	{
		bool bDirty = false;
		XMVECTOR Vec = XMLoadFloat3(&m_Vector);

		float Value[3];
		Value[0] = XMVectorGetX(Vec);
		Value[1] = XMVectorGetY(Vec);
		Value[2] = XMVectorGetZ(Vec);

		ImGui::PushID( id.c_str() );
		if (ImGui::DragFloat3(Label.c_str(), Value, 0.3f, 0, 0, "%.3f"))
		{
			XMStoreFloat3( &m_Vector, XMVectorSet( Value[0], Value[1], Value[2], 0 ) );
			bDirty = true;
		}
		ImGui::PopID();

		return bDirty;
	}
#endif
}