#include "DrnPCH.h"
#include "Vector.h"
#include "Runtime/Misc/Parse.h"

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

	std::string Vector::ToString()
	{
		return std::format("(X={:.6f},Y={:.6f},Z={:.6f})", m_Vector.x, m_Vector.y, m_Vector.z);
	}

	bool Vector::FromString( const std::string& Str )
	{
		float X, Y, Z;
		const bool Successful = Parse::Value(Str, "X=", X) && Parse::Value(Str, "Y=", Y) && Parse::Value(Str, "Z=", Z); 
		if (Successful)
		{
			*this = Vector(X, Y, Z);
		}

		return Successful;
	}

#if WITH_EDITOR
	bool Vector::Draw(const std::string& id, const std::string& Label, EParameterPopupContext PopupOptions)
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

		if (EnumHasAnyFlags(PopupOptions, EParameterPopupContext::CopyPaste))
		{
			if (ImGui::BeginPopupContextItem(id.c_str()))
			{
				if (EnumHasAnyFlags(PopupOptions, EParameterPopupContext::Copy))
				{
					if (ImGui::Button("Copy"))
					{
						std::wstring Clipboard = StringHelper::s2ws(ToString());
						ApplicationMisc::ClipboardCopy(Clipboard);
					}
				}

				if ( EnumHasAnyFlags( PopupOptions, EParameterPopupContext::Paste ) )
				{
					if (ImGui::Button("Paste"))
					{
						std::wstring Clipboard = L"";
						ApplicationMisc::ClipboardPaste(Clipboard);
						bDirty |= FromString(StringHelper::ws2s(Clipboard));
					}
				}

				if (ImGui::Button("ConvertFromUe"))
				{
					*this = Vector(m_Vector.x, m_Vector.z, -m_Vector.y) * 0.01f;
					bDirty = true;
				}

				ImGui::EndPopup();
			}
		}

		ImGui::PopID();

		return bDirty;
	}
#endif
}