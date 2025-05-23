#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "Runtime/Renderer/Material/Material.h"
#include "Runtime/Core/AssetManager.h"



namespace Drn
{
	class MaterialData : public Serializable
	{
	public:
		MaterialData()
			: m_Material(DEFAULT_MATERIAL_PATH)
		{
		};

		virtual void Serialize( Archive& Ar ) override;

		std::string m_Name;
		AssetHandle<Material> m_Material;

	};

	class MaterialOverrideData : public MaterialData
	{
	public:

		MaterialOverrideData()
			: MaterialData()
		{
		}

		virtual void Serialize( Archive& Ar ) override;
		bool m_Overriden = false;

#if WITH_EDITOR
		void Draw(StaticMeshComponent* MC, uint32 MaterialIndex);
#endif
	};
}