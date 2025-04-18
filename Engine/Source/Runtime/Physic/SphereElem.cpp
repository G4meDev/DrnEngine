#include "DrnPCH.h"
#include "SphereElem.h"

namespace Drn
{
	void SphereElem::Serialize( Archive& Ar )
	{
		ShapeElem::Serialize(Ar);

		if (Ar.IsLoading())
		{
			Ar >> Center >> Radius;
		}

		else
		{
			Ar << Center << Radius;
		}
	}

}