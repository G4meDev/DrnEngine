#include "DrnPCH.h"
#include "PhysicalMaterial.h"

#include "Editor/AssetPreview/AssetPreviewPhysicalMaterialGuiLayer.h"

namespace Drn
{
	PhysicalMaterial::PhysicalMaterial( const std::string& Path )
		: Asset(Path)
		, Friction(0.7f)
		, StaticFriction(0.0f)
		, Restitution(0.3f)
		, bOverrideFrictionCombineMode(false)
		, bOverrideRestitutionCombineMode(false)
	{
		Load();
	}

#if WITH_EDITOR
	PhysicalMaterial::PhysicalMaterial( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, Friction(0.7f)
		, StaticFriction(0.0f)
		, Restitution(0.3f)
		, bOverrideFrictionCombineMode(false)
		, bOverrideRestitutionCombineMode(false)
	{
		Save();
	}
#endif

	PhysicalMaterial::~PhysicalMaterial()
	{
		
	}

	void PhysicalMaterial::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Friction;
			Ar >> StaticFriction;
			Ar >> *(uint8*)&FrictionCombineMode;
			Ar >> bOverrideFrictionCombineMode;

			Ar >> Restitution;
			Ar >> *(uint8*)&RestitutionCombineMode;
			Ar >> bOverrideRestitutionCombineMode;

			Ar >> *(uint8*)&SurfaceType;
		}

		else
		{
			Ar << Friction;
			Ar << StaticFriction;
			Ar << (uint8)FrictionCombineMode;
			Ar << bOverrideFrictionCombineMode;

			Ar << Restitution;
			Ar << (uint8)RestitutionCombineMode;
			Ar << bOverrideRestitutionCombineMode;

			Ar << (uint8)SurfaceType;
		}
	}

	EAssetType PhysicalMaterial::GetAssetType() { return EAssetType::PhysicalMaterial; }

#if WITH_EDITOR

	void PhysicalMaterial::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewPhysicalMaterialGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void PhysicalMaterial::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}

#endif

}