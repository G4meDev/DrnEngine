#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Physic/BodySetup.h"
#include "Runtime/Renderer/InputLayout.h"
#include "MeshTypes.h"

#include <wrl.h>

LOG_DECLARE_CATEGORY(LogStaticMesh)

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;

	struct StaticMeshVertexData : public Serializable
	{
		StaticMeshVertexData()
		{};

		virtual void Serialize(Archive& Ar) override;

		uint64 GetVertexCount() const { VertexCount; }

		const std::vector<Vector>& GetPositions() const { return Positions; }
		const std::vector<uint32>& GetIndices() const { return Indices; }

		bool HasNormals() const { return Normals.size() > 0; }
		const std::vector<Vector>& GetNormals() const { return Normals; }

		bool HasTangents() const { return Tangents.size() > 0; }
		const std::vector<Vector>& GetTangents() const { return Tangents; }

		bool HasBitTangents() const { return BitTangents.size() > 0; }
		const std::vector<Vector>& GetBitTangents() const { return BitTangents; }

	private:

		uint64 VertexCount;

		std::vector<Vector> Positions;
		std::vector<uint32> Indices;

		std::vector<Vector> Normals;
		std::vector<Vector> Tangents;
		std::vector<Vector> BitTangents;
		std::vector<Vector4> Color;

		std::vector<Vector2> UV_1;
		std::vector<Vector2> UV_2;
		std::vector<Vector2> UV_3;
		std::vector<Vector2> UV_4;
		std::vector<Vector2> UV_5;
		std::vector<Vector2> UV_6;
		std::vector<Vector2> UV_7;
		std::vector<Vector2> UV_8;

		friend class AssetImporterStaticMesh;
	};

	struct StaticMeshSlotData : public Serializable
	{
	public:
		StaticMeshSlotData();
		~StaticMeshSlotData();

		ID3DBlob* VertexBufferBlob = nullptr;
		ID3DBlob* IndexBufferBlob = nullptr;
		uint8 MaterialIndex = 0;

		class VertexBuffer* m_VertexBuffer; // TODO: swap with pos only
		class IndexBuffer* m_IndexBuffer;

		class VertexBuffer* m_PositionOnlyVertexBuffer;

#if WITH_EDITOR
		uint64 m_VertexCount = 0;

		std::vector<Vector> Positions;
		std::vector<Vector> Normals;
		std::vector<Vector> Tangents;
		std::vector<Vector> BitTangents;

		StaticMeshVertexData VertexData;

		void UnpackVerticesData();
#endif

		virtual void Serialize(Archive& Ar) override;
		
		inline void ReleaseBlobs()
		{
			if (VertexBufferBlob)
			{
				VertexBufferBlob->Release();
				VertexBufferBlob = nullptr;
			}
			if (IndexBufferBlob)
			{
				IndexBufferBlob->Release();
				IndexBufferBlob = nullptr;
			}
		}

		inline void ReleaseBuffers();

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList ) const;
		void BindAndDrawPrepass( ID3D12GraphicsCommandList2* CommandList ) const; // TODO: remove
	};

	struct StaticMeshData : public Serializable
	{
	public:
		StaticMeshData() {};

		std::vector<StaticMeshSlotData> MeshesData;
		std::vector<MaterialData> Materials;

		virtual void Serialize( Archive& Ar ) override;
	};

	class StaticMesh : public Asset
	{
	public:
		
		StaticMesh(const std::string& InPath);
		virtual ~StaticMesh();

#if WITH_EDITOR
		StaticMesh(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual void Serialize(Archive& Ar) override;

		void InitResources( ID3D12GraphicsCommandList2* CommandList );
		void UploadResources( ID3D12GraphicsCommandList2* CommandList );

		inline BodySetup* GetBodySetup() { return &m_BodySetup; }

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

		AssetHandle<Material> GetMaterialAtIndex(uint32 Index);

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::StaticMesh; }

	protected:

#if WITH_EDITOR
		void Import();
#endif

		StaticMeshData Data;
		bool m_RenderStateDirty;

		friend class Renderer;
		friend class SceneRenderer;

	private:

		std::string m_SourcePath;

		float ImportScale = 1.0f;

		BodySetup m_BodySetup;

		bool m_ImportNormals = true;
		bool m_ImportTangents = true;
		bool m_ImportBitTangents = true;
		bool m_ImportColor = false;
		uint8 m_ImportUVs = 1;

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewStaticMeshGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewStaticMeshGuiLayer;
		friend class AssetImporterStaticMesh;
		friend class StaticMeshSceneProxy;
		friend class StaticMeshComponent;
		friend class Editor;
	};
}