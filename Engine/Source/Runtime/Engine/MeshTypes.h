#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "Runtime/Renderer/Material/Material.h"
#include "Runtime/Core/AssetManager.h"

namespace Drn
{
	enum class EMaterialType : uint8
	{
		Material,
		MaterialInstance,
		MaterialInstanceDynamic,
	};

	class MaterialSlot : public Serializable
	{
	public:
		MaterialSlot()
			: Type(EMaterialType::Material)
			, MaterialHandle(DEFAULT_MATERIAL_PATH)
		{}

		MaterialSlot(AssetHandle<Material> InMaterial)
			: Type(EMaterialType::Material)
			, MaterialHandle(InMaterial)
		{}

		virtual ~MaterialSlot() {}

		virtual void Serialize( Archive& Ar ) override;

		Material* GetMaterial() const;
		MaterialInterface* GetMaterialInterface() const;

		std::string GetMaterialPath() const;

		void LoadChecked();
		void Load();
		bool IsValid() const;

		std::string GetMaterialName() const;

	private:
		EMaterialType Type;
		AssetHandle<Material> MaterialHandle;
		//union
		//{
		//	AssetHandle<Material> MaterialHandle;
		//	//AssetHandle<MaterialInstance> MaterialInstanceHandle;
		//};
	};

	class MaterialData : public Serializable
	{
	public:
		MaterialData()
		{};

		virtual ~MaterialData() {}

		virtual void Serialize( Archive& Ar ) override;

		std::string m_Name;
		MaterialSlot m_MaterialSlot;
	};

	class MaterialOverrideData : public MaterialData
	{
	public:

		MaterialOverrideData()
			: MaterialData()
		{
		}

		virtual ~MaterialOverrideData() {}

		virtual void Serialize( Archive& Ar ) override;
		bool m_Overriden = false;

#if WITH_EDITOR
		void Draw(StaticMeshComponent* MC, uint32 MaterialIndex);
#endif
	};
}