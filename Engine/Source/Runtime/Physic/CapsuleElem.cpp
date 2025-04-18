#include "DrnPCH.h"
#include "CapsuleElem.h"

namespace Drn
{
	void CapsuleElem::Serialize( Archive& Ar )
	{
		ShapeElem::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Center >> Rotation >> Radius >> Length;
		}

		else
		{
			Ar << Center << Rotation << Radius << Length;
		}
	}

}