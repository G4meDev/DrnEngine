#include "DrnPCH.h"
#include "BoxElem.h"

namespace Drn
{
	void BoxElem::Serialize( Archive& Ar )
	{
		ShapeElem::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Center >> Rotation >> Extent;
		}

		else
		{
			Ar << Center << Rotation << Extent;
		}
	}

}