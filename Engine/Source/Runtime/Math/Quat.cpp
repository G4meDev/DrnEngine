#include "DrnPCH.h"
#include "Quat.h"
#include "Runtime/Misc/Parse.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Quat Quat::Identity = Quat();

	//Rotator Quat::ToRotator() const
	//{
	//	float X = m_Vector.x;
	//	float Y = m_Vector.y;
	//	float Z = m_Vector.z;
	//	float W = m_Vector.w;
	//
	//	const float SingularityTest = Z*X-W*Y;
	//	const float YawY = 2.f*(W*Z+X*Y);
	//	const float YawX = (1.f-2.f*(Y*Y + Z*Z));
	//
	//	const float SINGULARITY_THRESHOLD = 0.4999995f;
	//	const float RAD_TO_DEG = (180.f)/XM_PI;
	//	Rotator RotatorFromQuat = Rotator();
	//
	//	if (SingularityTest < -SINGULARITY_THRESHOLD)
	//	{
	//		RotatorFromQuat.Pitch = -90.f;
	//		RotatorFromQuat.Yaw = std::atan2(YawY, YawX) * RAD_TO_DEG;
	//		RotatorFromQuat.Roll = Rotator::NormalizeAxis(-RotatorFromQuat.Yaw - (2.f * std::atan2(X, W) * RAD_TO_DEG));
	//	}
	//	else if (SingularityTest > SINGULARITY_THRESHOLD)
	//	{
	//		RotatorFromQuat.Pitch = 90.f;
	//		RotatorFromQuat.Yaw = std::atan2(YawY, YawX) * RAD_TO_DEG;
	//		RotatorFromQuat.Roll = Rotator::NormalizeAxis(RotatorFromQuat.Yaw - (2.f * std::atan2(X, W) * RAD_TO_DEG));
	//	}
	//	else
	//	{
	//		RotatorFromQuat.Pitch = std::asin(2.f*(SingularityTest)) * RAD_TO_DEG;
	//		RotatorFromQuat.Yaw = std::atan2(YawY, YawX) * RAD_TO_DEG;
	//		RotatorFromQuat.Roll = std::atan2(-2.f*(W*X+Y*Z), (1.f-2.f*(X*X + Y*Y))) * RAD_TO_DEG;
	//	}
	//
	//	return RotatorFromQuat;
	//}

	Quat Quat::CubeFaceOrientation[6] =
	{
		Quat(Vector::UpVector		, XM_PIDIV2),	// direction: right		, up: up
		Quat(Vector::UpVector		, -XM_PIDIV2),	// direction: left		, up: up
		Quat(Vector::LeftVector		, XM_PIDIV2),	// direction: up		, up: backward
		Quat(Vector::LeftVector		, -XM_PIDIV2),	// direction: down		, up: forward
		Quat(Vector::ForwardVector	, 0),			// direction: forward	, up: up
		Quat(Vector::UpVector		, XM_PI),		// direction: backward	, up: up
	};

#if WITH_EDITOR
	bool Quat::Draw(const std::string& id, const std::string& Label, EParameterPopupContext PopupOptions)
	{
		bool bDirty = false;

		float Value[4] = { GetX(), GetY(), GetZ(), GetW() };
		ImGui::PushID(id.c_str());
		if ( ImGui::DragFloat4( Label.c_str(), Value, 0.3f, 0, 0, "%.3f" ) )
		{
			XMStoreFloat4(&m_Vector, XMQuaternionNormalize( XMVectorSet( Value[0], Value[1], Value[2], Value[3] ) ));
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

				ImGui::EndPopup();
			}
		}

		ImGui::PopID();

		if (bDirty)
		{
			*this = GetNormalized();
		}

		return bDirty;
	}

	std::string Quat::ToString()
	{
		return std::format("(X={:.6f},Y={:.6f},Z={:.6f},W={:.6f})", m_Vector.x, m_Vector.y, m_Vector.z, m_Vector.w);
	}

	bool Quat::FromString(const std::string& Str)
	{
		{
			float X, Y, Z, W;
			const bool Successful = Parse::Value(Str, "X=", X) && Parse::Value(Str, "Y=", Y) && Parse::Value(Str, "Z=", Z) && Parse::Value(Str, "W=", W);
			if (Successful)
			{
				*this = Quat(X, Y, Z, W);
				return true;
			}
		}

		{
			float Pitch, Roll, Yaw;
			const bool Successful = Parse::Value(Str, "Pitch=", Pitch) && Parse::Value(Str, "Roll=", Roll) && Parse::Value(Str, "Yaw=", Yaw);
			if (Successful)
			{
				Roll = Math::DegreesToRadians(Roll);
				Pitch = Math::DegreesToRadians(Pitch);
				Yaw = Math::DegreesToRadians(Yaw);

				*this = Quat(Pitch, -Roll, Yaw);
				return true;
			}
		}

		return false;
	}

#endif
}