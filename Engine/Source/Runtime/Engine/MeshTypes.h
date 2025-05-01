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

		std::string m_Name;
		AssetHandle<Material> m_Material;

		virtual void Serialize( Archive& Ar ) override;
	};
}