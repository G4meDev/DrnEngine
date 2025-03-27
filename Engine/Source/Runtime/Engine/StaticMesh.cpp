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

		RenderProxies.clear();

		uint8 Type;
		Ar >> Type;

		std::string Source;
		Ar >> Source;

		Data.Serialize(Ar);

		for (auto& Mesh : Data.MeshesData)
		{
			StaticMeshRenderProxy RP;

			RP.VertexData = Mesh.VertexData;
			RP.IndexData = Mesh.IndexData;

			RenderProxies.push_back(RP);
		}
	}

	void StaticMesh::UploadResources( dx12lib::CommandList* CommandList )
	{
		for (auto& Proxy : RenderProxies)
		{
			Proxy.VertexBuffer = CommandList->CopyVertexBuffer( Proxy.VertexData.size(), sizeof(StaticMeshVertexBuffer), Proxy.VertexData.data());
			Proxy.IndexBuffer = CommandList->CopyIndexBuffer( Proxy.IndexData.size(), DXGI_FORMAT_R32_UINT, Proxy.IndexData.data());
		}
	}

// ----------------------------------------------------------------------------------------------------------

	void StaticMeshData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			MeshesData.clear();
			Materials.clear();

			uint8 size;
			Ar >> size;

			for (int i = 0; i < size; i++)
			{
				StaticMeshSlotData M;
				M.Serialize(Ar);

				MeshesData.push_back(M);
			}
		}

		else
		{
			uint8 size = MeshesData.size();
			Ar << size;

			for (int i = 0; i < size; i++)
			{
				MeshesData[i].Serialize(Ar);
			}
		}
		
	}

	void StaticMeshSlotData::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			std::vector<char> Buffer;

			Ar >> Buffer;
			int size =  Buffer.size() / sizeof( StaticMeshVertexBuffer );
			VertexData.resize(size);
			std::memcpy( VertexData.data(), Buffer.data(), Buffer.size() );

			Ar >> Buffer;
			size = Buffer.size() / sizeof(uint32);
			IndexData.resize(size);
			std::memcpy( IndexData.data(), Buffer.data(), Buffer.size() );

			Ar >> Stride;
			Ar >> MaterialIndex;
		}
		else
		{
			uint64 size = VertexData.size() * sizeof( StaticMeshVertexBuffer );
			std::vector<char> buffer( size );
			std::memcpy( buffer.data(), VertexData.data(), size );
			Ar << buffer;

			size = IndexData.size() * sizeof( uint32 );
			buffer.resize( size );
			std::memcpy( buffer.data(), IndexData.data(), size );
			Ar << buffer;

			Ar << Stride;
			Ar << MaterialIndex;
		}
	}
}