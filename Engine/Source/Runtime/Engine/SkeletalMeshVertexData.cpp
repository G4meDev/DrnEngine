#include "DrnPCH.h"
#include "SkeletalMeshVertexData.h"

namespace Drn
{
	Archive& operator<<(Archive& Ar, const FloatUnorm8& Data)
	{
		Ar << Data.Value;
		return Ar;
	}

	Archive& operator>>(Archive& Ar, FloatUnorm8& Data)
	{
		Ar >> Data.Value;
		return Ar;
	}

	void SkeletalMeshVertexData::Serialize( Archive& Ar )
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
			Ar >> Colors;
			
			Ar >> UV_1;
			Ar >> UV_2;
			Ar >> UV_3;
			Ar >> UV_4;

			Ar >> BoneIndices;
			Ar.operator>> <uint64>(BoneWeights);
			//Ar >> BoneWeights;

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
			Ar << Colors;
			
			Ar << UV_1;
			Ar << UV_2;
			Ar << UV_3;
			Ar << UV_4;

			Ar << BoneIndices;
			Ar.operator<< <uint64>(BoneWeights);
			//Ar << <uint64>(BoneWeights);
		}
	}

}