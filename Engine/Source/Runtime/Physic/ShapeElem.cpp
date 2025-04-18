#include "DrnPCH.h"
#include "ShapeElem.h"

namespace Drn
{
	void ShapeElem::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			uint8 ArType;
			Ar >> ArType;
			Type = static_cast<EAggCollisionShape>(ArType);
		}
		else
		{
			Ar << static_cast<uint8>(Type);
		}
	}

}