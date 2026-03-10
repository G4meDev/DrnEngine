#include "DrnPCH.h"
#include "Quat.h"

#if WITH_EDITOR
#include <imgui.h>
#endif

namespace Drn
{
	Quat Quat::Identity = Quat();

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
	bool Quat::Draw(const std::string& id, const std::string& Label)
	{
		bool bDirty = false;

		float Value[4] = { GetX(), GetY(), GetZ(), GetW() };
		ImGui::PushID(id.c_str());
		if ( ImGui::DragFloat4( Label.c_str(), Value, 0.3f, 0, 0, "%.3f" ) )
		{
			XMStoreFloat4(&m_Vector, XMQuaternionNormalize( XMVectorSet( Value[0], Value[1], Value[2], Value[3] ) ));
			bDirty = true;
		}
		ImGui::PopID();

		return bDirty;
	}
#endif
}