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

		virtual void Serialize( Archive& Ar ) override;
		bool m_Overriden = false;

#if WITH_EDITOR
		void Draw(StaticMeshComponent* MC);
#endif
	};
}