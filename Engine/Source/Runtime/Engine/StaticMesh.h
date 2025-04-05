#pragma once

#include "ForwardTypes.h"
//#include "Runtime/Core/Serializable.h"
#include "Runtime/Core/Asset.h"

LOG_DECLARE_CATEGORY(LogStaticMesh)

namespace Drn
{
	class AssetPreviewStaticMeshGuiLayer;

	struct StaticMeshSlotData : public Serializable
	{
	public:
		StaticMeshSlotData() {};

		//std::vector<float>  VertexData;
		std::vector<StaticMeshVertexBuffer> VertexData;
		std::vector<uint32> IndexData;

		uint8 Stride;

		uint8 MaterialIndex;

		virtual void Serialize(Archive& Ar) override;
	};

	struct StaticMeshData : public Serializable
	{
	public:
		StaticMeshData() {};

		std::vector<StaticMeshSlotData> MeshesData;
		std::vector<std::string> Materials;

		virtual void Serialize( Archive& Ar ) override;
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

	struct StaticMeshRenderProxy
	{
	public:
		StaticMeshRenderProxy(){};

		std::shared_ptr<dx12lib::VertexBuffer> VertexBuffer = nullptr;
		std::shared_ptr<dx12lib::IndexBuffer>  IndexBuffer  = nullptr;

		std::vector<StaticMeshVertexBuffer> VertexData;
		std::vector<uint32> IndexData;
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

	protected:

#if WITH_EDITOR
		void Import();
#endif

		virtual EAssetType GetAssetType() override;

		StaticMeshData Data;
		std::vector<StaticMeshRenderProxy> RenderProxies;

		bool m_LoadedOnGPU;

		friend class Renderer;
		friend class SceneRenderer;

	private:

		std::string m_SourcePath;

		float ImportScale = 1.0f;

		bool  ImportNormal    = false;
		bool  ImportTangent   = false;
		bool  ImportBiTangent = false;
		bool  ImportColor     = true;
		uint8 ImportUvCount   = 0;

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		AssetPreviewStaticMeshGuiLayer* GuiLayer = nullptr;
#endif

		friend class AssetPreviewStaticMeshGuiLayer;
		friend class AssetImporterStaticMesh;
		friend class Editor;
	};
}