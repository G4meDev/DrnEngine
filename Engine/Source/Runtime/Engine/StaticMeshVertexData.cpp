#include "DrnPCH.h"
#include "StaticMeshVertexData.h"

namespace Drn
{
	void StaticMeshVertexData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> Positions;
			Ar >> Indices;
			
			Ar >> Normals;
			Ar >> Tangents;
			Ar >> BitTangents;
			Ar >> Color;
			
			Ar >> UV_1;
			Ar >> UV_2;
			Ar >> UV_3;
			Ar >> UV_4;

			VertexCount = Positions.size();
		}

		else
		{
			Ar << Positions;
			Ar << Indices;
			
			Ar << Normals;
			Ar << Tangents;
			Ar << BitTangents;
			Ar << Color;
			
			Ar << UV_1;
			Ar << UV_2;
			Ar << UV_3;
			Ar << UV_4;
		}
	}
}