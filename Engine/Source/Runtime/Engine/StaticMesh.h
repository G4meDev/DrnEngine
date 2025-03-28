#pragma once

#include "ForwardTypes.h"
#include "Runtime/Core/Serializable.h"
#include "Runtime/Core/Asset.h"

LOG_DECLARE_CATEGORY(LogStaticMesh)

namespace Drn
{
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


	class StaticMesh : public Serializable, public Asset
	{
	public:
		StaticMesh(const std::string& Path);

		virtual void Serialize(Archive& Ar) override;

		void UploadResources( dx12lib::CommandList* CommandList );

	protected:

		virtual void Load() override;

		StaticMeshData Data;
		std::vector<StaticMeshRenderProxy> RenderProxies;

		friend class Renderer;
		friend class SceneRenderer;

	private:
	};
}