#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class Material;

	//struct MaterialParameterInfo
	//{
	//
	//};



	class MaterialInterface
	{
	public:
		virtual AssetHandle<Material> GetMaterial() const = 0;
		virtual bool IsDependent(MaterialInterface* OtherMaterial) const = 0;
		virtual bool IsTwoSided() const = 0;


	};
}