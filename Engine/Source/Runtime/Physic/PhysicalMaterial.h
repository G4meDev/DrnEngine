#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"

namespace Drn
{
	enum class EFrictionCombineMode : uint8
	{
		Average		= 0,
		Min			= 1,
		Multiply	= 2,
		Max			= 3
	};

	enum class EPhysicalSurface : uint8
	{
		SurfaceType_Default = 0,
		SurfaceType1,
		SurfaceType2,
		SurfaceType3,
		SurfaceType4,
		SurfaceType5,
		SurfaceType6,
		SurfaceType7,
		SurfaceType8,
		SurfaceType9,
		SurfaceType10,
		SurfaceType11,
		SurfaceType12,
		SurfaceType13,
		SurfaceType14,
		SurfaceType15,
		SurfaceType16,
		SurfaceType17,
		SurfaceType18,
		SurfaceType19,
		SurfaceType20,
		SurfaceType21,
		SurfaceType22,
		SurfaceType23,
		SurfaceType24,
		SurfaceType25,
		SurfaceType26,
		SurfaceType27,
		SurfaceType28,
		SurfaceType29,
		SurfaceType30,
		SurfaceType31,
		SurfaceType32,
		SurfaceType33,
		SurfaceType34,
		SurfaceType35,
		SurfaceType36,
		SurfaceType37,
		SurfaceType38,
		SurfaceType39,
		SurfaceType40,
		SurfaceType41,
		SurfaceType42,
		SurfaceType43,
		SurfaceType44,
		SurfaceType45,
		SurfaceType46,
		SurfaceType47,
		SurfaceType48,
		SurfaceType49,
		SurfaceType50,
		SurfaceType51,
		SurfaceType52,
		SurfaceType53,
		SurfaceType54,
		SurfaceType55,
		SurfaceType56,
		SurfaceType57,
		SurfaceType58,
		SurfaceType59,
		SurfaceType60,
		SurfaceType61,
		SurfaceType62,
	};

	class PhysicalMaterial : public Asset
	{
	public:
		PhysicalMaterial(const std::string& Path);
		virtual ~PhysicalMaterial();

#if WITH_EDITOR
		PhysicalMaterial(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual void Serialize(Archive& Ar) override;

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::PhysicalMaterial; }

		inline physx::PxMaterial* const GetMaterial() { return MaterialHandle; }

		void UpdatePhysicParams();

		inline EPhysicalSurface GetSurfaceType() const { return SurfaceType; }

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		class AssetPreviewPhysicalMaterialGuiLayer* GuiLayer = nullptr;
#endif

	private:
		float Friction;
		float StaticFriction;
		EFrictionCombineMode FrictionCombineMode;
		bool bOverrideFrictionCombineMode;

		float Restitution;
		EFrictionCombineMode RestitutionCombineMode;
		bool bOverrideRestitutionCombineMode;

		// 6bit
		EPhysicalSurface SurfaceType;

		physx::PxMaterial* MaterialHandle;
		PhysicUserData UserData;

		friend class AssetPreviewPhysicalMaterialGuiLayer;
	};
}