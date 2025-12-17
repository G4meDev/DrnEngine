#include "DrnPCH.h"
#include "StaticMeshVertexData.h"

namespace Drn
{
	void StaticMeshVertexData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> bUse4BitIndices;
			if (bUse4BitIndices)
			{
				Ar >> Indices_32;
			}
			else
			{
				Ar >> Indices_16;
			}

			Ar >> Positions;
			Ar >> Normals;
			Ar >> Tangents;
			Ar >> BitTangents;
			Ar >> Colors;
			
			Ar >> UV_1;
			Ar >> UV_2;
			Ar >> UV_3;
			Ar >> UV_4;

			VertexCount = Positions.size();
			IndexCount = Use4BitIndices() ? Indices_32.size() : Indices_16.size();
		}

		else
		{
			Ar << bUse4BitIndices;
			if (bUse4BitIndices)
			{
				Ar << Indices_32;
			}
			else
			{
				Ar << Indices_16;
			}

			Ar << Positions;
			Ar << Normals;
			Ar << Tangents;
			Ar << BitTangents;
			Ar << Colors;
			
			Ar << UV_1;
			Ar << UV_2;
			Ar << UV_3;
			Ar << UV_4;
		}
	}
}