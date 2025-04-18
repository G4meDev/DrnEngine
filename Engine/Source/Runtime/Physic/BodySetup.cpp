#include "DrnPCH.h"
#include "BodySetup.h"

namespace Drn
{
	void BodySetup::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			m_AggGeo.Serialize(Ar);
		}
		else
		{
			m_AggGeo.Serialize(Ar);
		}
	}

}