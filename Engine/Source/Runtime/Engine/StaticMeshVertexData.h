#pragma once

namespace Drn
{
	struct LowPrecisionNormalizedVector
	{
		union { struct { int8 R, G, B, A; }; uint32 AlignmentDummy; };
	};

	struct StaticMeshVertexData : public Serializable
	{
		StaticMeshVertexData()
			: VertexCount(0)
			, bUse4BitIndices(true)
		{};

		virtual void Serialize(Archive& Ar) override;

		uint64 GetVertexCount() const { return VertexCount; }
		uint64 GetIndexCount() const { return IndexCount; }
		uint64 GetPrimitiveCount() const { drn_check(IndexCount > 0); return IndexCount / 3; }

		bool Use4BitIndices() const { return bUse4BitIndices; }
		uint32 GetIndexBufferStride() const { return bUse4BitIndices ? 4 : 2; }
		uint32 GetIndexBufferByteSize() const { return GetIndexBufferStride() * IndexCount; }
		void* GetIndexBufferPtr() const { return Use4BitIndices() ? (void*)Indices_32.data() : (void*)Indices_16.data(); }

		const std::vector<Vector>& GetPositions() const { return Positions; }
		const std::vector<uint32>& GetIndices_32() const { return Indices_32; }
		const std::vector<uint16>& GetIndices_16() const { return Indices_16; }

		bool HasNormals() const { return Normals.size() > 0; }
		const std::vector<uint32>& GetNormals() const { return Normals; }

		bool HasTangents() const { return Tangents.size() > 0; }
		const std::vector<uint32>& GetTangents() const { return Tangents; }

		bool HasBitTangents() const { return BitTangents.size() > 0; }
		const std::vector<uint32>& GetBitTangents() const { return BitTangents; }

		bool HasColors() const { return Colors.size() > 0; }
		const std::vector<Color>& GetColor() const { return Colors; }

		bool HasUV1() const { return UV_1.size() > 0; }
		const std::vector<Vector2Half>& GetUV1() const { return UV_1; }

		bool HasUV2() const { return UV_2.size() > 0; }
		const std::vector<Vector2Half>& GetUV2() const { return UV_2; }

		bool HasUV3() const { return UV_3.size() > 0; }
		const std::vector<Vector2Half>& GetUV3() const { return UV_3; }

		bool HasUV4() const { return UV_4.size() > 0; }
		const std::vector<Vector2Half>& GetUV4() const { return UV_4; }

	private:

		uint64 VertexCount;
		uint64 IndexCount;

		std::vector<Vector> Positions;
		std::vector<uint32> Indices_32;
		std::vector<uint16> Indices_16;

		std::vector<uint32> Normals;
		std::vector<uint32> Tangents;
		std::vector<uint32> BitTangents;
		std::vector<Color> Colors;

		std::vector<Vector2Half> UV_1;
		std::vector<Vector2Half> UV_2;
		std::vector<Vector2Half> UV_3;
		std::vector<Vector2Half> UV_4;

		bool bUse4BitIndices;

		friend class AssetImporterStaticMesh;
	};
}