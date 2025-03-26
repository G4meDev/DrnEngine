#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"

LOG_DECLARE_CATEGORY(LogStaticMesh)

namespace Drn
{
	struct StaticMeshVertexData : Serializable
	{
	public:
		StaticMeshVertexData(){};

		virtual void Serialize(Archive& Ar) override;

		float Pos_X;
		float Pos_Y;
		float Pos_Z;

		float Normal_X;
		float Normal_Y;
		float Normal_Z;

		float Tangent_X;
		float Tangent_Y;
		float Tangent_Z;

		float BiTangent_X;
		float BiTangent_Y;
		float BiTangent_Z;

		float Color_R;
		float Color_G;
		float Color_B;

		float U_1;
		float V_1;
		
		float U_2;
		float V_2;

		float U_3;
		float V_3;

		float U_4;
		float V_4;
	};

	struct MaterialData
	{
	public:
		MaterialData(){};

		std::string Name;
	};

	struct StaticMeshData : Serializable
	{
	public:
		StaticMeshData(){};

		virtual void Serialize(Archive& Ar) override;

		std::vector<StaticMeshVertexData> Vertices;
		std::vector<uint32> Indices;
		std::vector<StaticMeshVertexBuffer> VertexBuffer;

		uint8 MaterialIndex;
	};

	struct StaticMeshVertexBuffer
	{
	public:
		StaticMeshVertexBuffer(){};

		float Pos_X;
		float Pos_Y;
		float Pos_Z;

		float Color_R;
		float Color_G;
		float Color_B;
	};

	class StaticMesh : Serializable
	{
	public:
		StaticMesh(const std::string& Path);

		virtual void Serialize(Archive& Ar) override;

	protected:

		

	private:
	};
}