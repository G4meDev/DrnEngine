#include "DrnPCH.h"
#include "EditorConfig.h"

#if WITH_EDITOR

namespace Drn
{
	ImVec4 EditorConfig::AssetInputColor = ImVec4(0.3f, 0.5f, 0.9f, 1);

	const char* EditorConfig::Payload_AssetPath()
	{
		return "ASSET_PATH";
	}

	const char* EditorConfig::Payload_EditorSpawnable()
	{
		return "EDITOR_SPAWNABLE";
	}

}

#endif