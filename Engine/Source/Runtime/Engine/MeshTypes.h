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
		{
		};

		MaterialData(const MaterialData& Other)
		{
			m_Name = Other.m_Name;
			m_Material = Other.m_Material;
		};

		MaterialData& operator=(const MaterialData& Other)
		{
			m_Name = Other.m_Name;
			m_Material = Other.m_Material;

			return *this;
		}

		virtual void Serialize( Archive& Ar ) override;

		std::string m_Name;
		AssetHandle<Material> m_Material;

	};

	class MaterialOverrideData : public MaterialData
	{
	public:

		MaterialOverrideData(const std::string& InName)
		{
			m_Name = InName;
		}

		MaterialOverrideData(const MaterialOverrideData& Other)
			: MaterialData(Other)
		{
			m_Overriden = Other.m_Overriden;
		}

		MaterialOverrideData& operator=(const MaterialOverrideData& Other)
		{
			m_Name = Other.m_Name;
			m_Material = Other.m_Material;
			m_Overriden = Other.m_Overriden;

			return *this;
		}

		virtual void Serialize( Archive& Ar ) override;
		bool m_Overriden = false;

#if WITH_EDITOR
		void Draw(StaticMeshComponent* MC, uint32 MaterialIndex);
#endif
	};
}