#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Physic/BodySetup.h"
#include "Runtime/Renderer/InputLayout.h"
#include "MeshTypes.h"
#include "Runtime/Engine/StaticMeshVertexData.h"

#include <wrl.h>

LOG_DECLARE_CATEGORY(LogStaticMesh)

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;

	struct StaticMeshSlotData : public Serializable
	{
	public:
		StaticMeshSlotData();
		~StaticMeshSlotData();

		uint8 MaterialIndex = 0;

		StaticMeshVertexData VertexData;

		class StaticMeshVertexBuffer* m_StaticMeshVertexBuffer;
		class IndexBuffer* m_IndexBuffer;

		virtual void Serialize(Archive& Ar) override;

		inline void ReleaseBuffers();

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList ) const;
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