#pragma once

namespace Drn
{
	struct StaticMeshVertexData : public Serializable
	{
		StaticMeshVertexData()
			: VertexCount(0)
		{};

		virtual void Serialize(Archive& Ar) override;

		uint64 GetVertexCount() const { return VertexCount; }

		const std::vector<Vector>& GetPositions() const { return Positions; }
		const std::vector<uint32>& GetIndices() const { return Indices; }

		bool HasNormals() const { return Normals.size() > 0; }
		const std::vector<Vector>& GetNormals() const { return Normals; }

		bool HasTangents() const { return Tangents.size() > 0; }
		const std::vector<Vector>& GetTangents() const { return Tangents; }

		bool HasBitTangents() const { return BitTangents.size() > 0; }
		const std::vector<Vector>& GetBitTangents() const { return BitTangents; }

		bool HasColors() const { return Colors.size() > 0; }
		const std::vector<Color>& GetColor() const { return Colors; }

		bool HasUV1() const { return UV_1.size() > 0; }
		const std::vector<Vector2>& GetUV1() const { return UV_1; }

		bool HasUV2() const { return UV_2.size() > 0; }
		const std::vector<Vector2>& GetUV2() const { return UV_2; }

		bool HasUV3() const { return UV_3.size() > 0; }
		const std::vector<Vector2>& GetUV3() const { return UV_3; }

		bool HasUV4() const { return UV_4.size() > 0; }
		const std::vector<Vector2>& GetUV4() const { return UV_4; }

	private:

		uint64 VertexCount;

		std::vector<Vector> Positions;
		std::vector<uint32> Indices;

		std::vector<Vector> Normals;
		std::vector<Vector> Tangents;
		std::vector<Vector> BitTangents;
		std::vector<Color> Colors;

		std::vector<Vector2> UV_1;
		std::vector<Vector2> UV_2;
		std::vector<Vector2> UV_3;
		std::vector<Vector2> UV_4;

		friend class AssetImporterStaticMesh;
	};
}