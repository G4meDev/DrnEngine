#include "DrnPCH.h"
#include "PhysicalMaterial.h"

#include "Editor/AssetPreview/AssetPreviewPhysicalMaterialGuiLayer.h"

#define DEFAULT_FRICTION_COMBO EFrictionCombineMode::Average
#define DEFAULT_RESTITUTION_COMBO EFrictionCombineMode::Average

namespace Drn
{
	PhysicalMaterial::PhysicalMaterial( const std::string& Path )
		: Asset(Path)
		, Friction(0.7f)
		, StaticFriction(0.0f)
		, Restitution(0.3f)
		, bOverrideFrictionCombineMode(false)
		, bOverrideRestitutionCombineMode(false)
		, MaterialHandle(nullptr)
	{
		Load();

		MaterialHandle = PhysicManager::Get()->GetPhysics()->createMaterial(0, 0, 0);
		UpdatePhysicParams();
	}

#if WITH_EDITOR
	PhysicalMaterial::PhysicalMaterial( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, Friction(0.7f)
		, StaticFriction(0.0f)
		, Restitution(0.3f)
		, bOverrideFrictionCombineMode(false)
		, bOverrideRestitutionCombineMode(false)
		, MaterialHandle(nullptr)
	{
		Save();
	}
#endif

	PhysicalMaterial::~PhysicalMaterial()
	{
		if (MaterialHandle)
		{
			MaterialHandle->release();
		}
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

	void PhysicalMaterial::UpdatePhysicParams()
	{
		if (MaterialHandle)
		{
			MaterialHandle->setStaticFriction(StaticFriction);
			MaterialHandle->setDynamicFriction(Friction);
			MaterialHandle->setRestitution(std::clamp(Restitution, 0.0f, 1.0f));

			MaterialHandle->setFrictionCombineMode((physx::PxCombineMode::Enum)(bOverrideFrictionCombineMode ? FrictionCombineMode : DEFAULT_FRICTION_COMBO));
			MaterialHandle->setRestitutionCombineMode((physx::PxCombineMode::Enum)(bOverrideRestitutionCombineMode ? RestitutionCombineMode : DEFAULT_RESTITUTION_COMBO));
		}
	}

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