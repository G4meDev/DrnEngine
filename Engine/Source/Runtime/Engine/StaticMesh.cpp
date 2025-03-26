#include "DrnPCH.h"
#include "StaticMesh.h"

LOG_DEFINE_CATEGORY( LogStaticMesh, "StaticMesh" )

using namespace DirectX;

namespace Drn
{
	StaticMesh::StaticMesh(const std::string& Path)
	{
		Archive Ar = Archive(Path);
		Serialize(Ar);
	}

	void StaticMesh::Serialize( Archive& Ar )
	{
#if WITH_EDITOR

		if (!Ar.IsLoading())
		{
			LOG(LogStaticMesh, Error, "tring to save to a static mesh. if you want update a static mesh, try updating its asset.");
		}

#endif

		

	}

	void StaticMeshData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			std::vector<char> Buffer;
			
			Ar >> Buffer;
			int size =  Buffer.size() / sizeof( StaticMeshVertexData );
			Vertices.resize(size);
			std::memcpy( Vertices.data(), Buffer.data(), Buffer.size() );

			Ar >> Buffer;
			size = Buffer.size() / sizeof(uint32);
			Indices.resize(size);
			std::memcpy( Indices.data(), Buffer.data(), Buffer.size() );

			Ar >> Buffer;
			size = Buffer.size() / sizeof(StaticMeshVertexBuffer);
			VertexBuffer.resize(size);
			std::memcpy( VertexBuffer.data(), Buffer.data(), Buffer.size() );
		}

		else
		{
			uint64 size = Vertices.size() * sizeof(StaticMeshVertexData);
			std::vector<char> buffer(size);
			std::memcpy(buffer.data(), Vertices.data(), size);
			Ar << buffer;

			size = Indices.size() * sizeof(uint32);
			buffer.resize(size);
			std::memcpy(buffer.data(), Indices.data(), size);
			Ar << buffer;

			size = VertexBuffer.size() * sizeof( StaticMeshVertexBuffer);
			buffer.resize( size );
			std::memcpy( buffer.data(), VertexBuffer.data(), size );
			Ar << buffer;
		}
	}

	void StaticMeshVertexData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Ar >> Pos_X;
			Ar >> Pos_Y;
			Ar >> Pos_Z;

			Ar >> Normal_X;
			Ar >> Normal_Y;
			Ar >> Normal_Z;

			Ar >> Tangent_X;
			Ar >> Tangent_Y;
			Ar >> Tangent_Z;

			Ar >> BiTangent_X;
			Ar >> BiTangent_Y;
			Ar >> BiTangent_Z;

			Ar >> Color_R;
			Ar >> Color_G;
			Ar >> Color_B;

			Ar >> U_1;
			Ar >> V_1;

			Ar >> U_2;
			Ar >> V_2;

			Ar >> U_3;
			Ar >> V_3;

			Ar >> U_4;
			Ar >> V_4;
		}

		else
		{
			Ar << Pos_X;
			Ar << Pos_Y;
			Ar << Pos_Z;

			Ar << Normal_X;
			Ar << Normal_Y;
			Ar << Normal_Z;

			Ar << Tangent_X;
			Ar << Tangent_Y;
			Ar << Tangent_Z;

			Ar << BiTangent_X;
			Ar << BiTangent_Y;
			Ar << BiTangent_Z;

			Ar << Color_R;
			Ar << Color_G;
			Ar << Color_B;

			Ar << U_1;
			Ar << V_1;

			Ar << U_2;
			Ar << V_2;

			Ar << U_3;
			Ar << V_3;

			Ar << U_4;
			Ar << V_4;
		}
	}
}