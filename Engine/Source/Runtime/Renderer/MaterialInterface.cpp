#include "DrnPCH.h"
#include "MaterialInterface.h"
#include "Runtime/Renderer/Material/Material.h"

namespace Drn
{
	bool MaterialInterface::IsTwoSided() const
	{
		return GetMaterial()->IsTwoSided();
	}

}