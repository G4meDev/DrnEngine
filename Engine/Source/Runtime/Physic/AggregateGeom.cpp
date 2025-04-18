#include "DrnPCH.h"
#include "AggregateGeom.h"

namespace Drn
{
	void AggregateGeom::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			uint32 SphereCount	= 0;
			uint32 BoxCount		= 0;
			uint32 CapsuleCount = 0;
			uint32 ConvexCount	= 0;
			Ar >> SphereCount >> BoxCount >> CapsuleCount >> ConvexCount;

			EmptyElements();
			SphereElems.reserve(SphereCount);
			BoxElems.reserve(BoxCount);
			CapsuleElems.reserve(CapsuleCount);
			ConvexElems.reserve(ConvexCount);

			for (uint32 i = 0; i < SphereCount; i++)
			{
				SphereElems.push_back(Ar);
			}

			for (uint32 i = 0; i < BoxCount; i++)
			{
				BoxElems.push_back(Ar);
			}

			for (uint32 i = 0; i < CapsuleCount; i++)
			{
				CapsuleElems.push_back(Ar);
			}

			for (uint32 i = 0; i < ConvexCount; i++)
			{
				ConvexElems.push_back(Ar);
			}
		}
		else
		{
			uint32 SphereCount	= SphereElems.size();
			uint32 BoxCount		= BoxElems.size();
			uint32 CapsuleCount = CapsuleElems.size();
			uint32 ConvexCount	= 0;
			Ar << SphereCount << BoxCount << CapsuleCount << ConvexCount;
			
			for (uint32 i = 0; i < SphereCount; i++)
			{
				SphereElems[i].Serialize(Ar);
			}

			for (uint32 i = 0; i < BoxCount; i++)
			{
				BoxElems[i].Serialize(Ar);
			}

			for (uint32 i = 0; i < CapsuleCount; i++)
			{
				CapsuleElems[i].Serialize(Ar);
			}

			for (uint32 i = 0; i < ConvexCount; i++)
			{
				ConvexElems[i].Serialize(Ar);
			}
		}
	}

}