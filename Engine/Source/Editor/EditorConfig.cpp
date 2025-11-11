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

	Color EditorConfig::GetAssetTypeColor( EAssetType Type )
	{
		switch ( Type )
		{
		case EAssetType::StaticMesh: return Color::Green;
		case EAssetType::Level:	return Color::Silver;
		case EAssetType::Material: return Color::Blue;
		case EAssetType::Texture2D:
		case EAssetType::TextureVolume:
		case EAssetType::TextureCube: return Color::Yellow;
		case EAssetType::Undefined:
		default: return Color::Black;
		}
	}

	
	const AssetHandle<Texture2D>& EditorConfig::GetAssetTypeIcon( EAssetType Type )
	{
		switch ( Type )
		{
		case EAssetType::Level:				return CommonResources::Get()->m_AssetIcon_Level;
		case EAssetType::StaticMesh:		return CommonResources::Get()->m_AssetIcon_StaticMesh;
		case EAssetType::Material:			return CommonResources::Get()->m_AssetIcon_Material;
		case EAssetType::Texture2D:			return CommonResources::Get()->m_AssetIcon_Texture2D;
		case EAssetType::TextureVolume:		return CommonResources::Get()->m_AssetIcon_TextureVolume;
		case EAssetType::TextureCube:		return CommonResources::Get()->m_AssetIcon_TextureCube;
		case EAssetType::Undefined:
		default:							return CommonResources::Get()->m_AssetIcon_Default;
		}
	}

}

#endif