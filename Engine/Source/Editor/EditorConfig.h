#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include <imgui.h>

namespace Drn
{
	class EditorConfig
	{
	public:
		static ImVec4 AssetInputColor;

		const static char* Payload_AssetPath();
		const static char* Payload_EditorSpawnable();
	};
}

#endif