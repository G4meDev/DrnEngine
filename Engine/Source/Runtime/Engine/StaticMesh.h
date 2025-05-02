#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Asset.h"
#include "Runtime/Physic/BodySetup.h"
#include "Runtime/Renderer/VertexLayout.h"
#include "MeshTypes.h"

LOG_DECLARE_CATEGORY(LogStaticMesh)

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;

	struct StaticMeshSlotData : public Serializable
	{
	public:
		StaticMeshSlotData() {};
		~StaticMeshSlotData()
		{
			ReleaseBlobs();
		};

		ID3DBlob* VertexBufferBlob = nullptr;
		ID3DBlob* IndexBufferBlob = nullptr;
		uint8 MaterialIndex = 0;

		std::shared_ptr<dx12lib::VertexBuffer> VertexBuffer = nullptr;
		std::shared_ptr<dx12lib::IndexBuffer>  IndexBuffer  = nullptr;

		virtual void Serialize(Archive& Ar) override;

		inline void ReleaseBlobs()
		{
			if (VertexBufferBlob) VertexBufferBlob->Release();
			if (IndexBufferBlob) IndexBufferBlob->Release();
		}
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

		void UploadResources( dx12lib::CommandList* CommandList );

		inline BodySetup* GetBodySetup() { return &m_BodySetup; }

		inline bool IsLoadedOnGpu() const { return m_LoadedOnGPU; }

		AssetHandle<Material> GetMaterialAtIndex(uint32 Index);

	protected:

#if WITH_EDITOR
		void Import();
#endif

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::StaticMesh; }

		StaticMeshData Data;
		bool m_LoadedOnGPU;

		friend class Renderer;
		friend class SceneRenderer;

	private:

		std::string m_SourcePath;

		float ImportScale = 1.0f;

		BodySetup m_BodySetup;

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