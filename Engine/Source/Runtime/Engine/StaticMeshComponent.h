#pragma once

#include "PrimitiveComponent.h"
#include "ForwardTypes.h"

using namespace DirectX;

namespace Drn
{
	struct VertexPosColor
	{
		XMFLOAT3 Position;
		XMFLOAT3 Color;
	};


	class StaticMeshComponent : public PrimitiveComponent
	{
	public:

		void UploadResources(dx12lib::CommandList* CommandList);

	protected:

		std::shared_ptr<dx12lib::VertexBuffer> m_VertexBuffer = nullptr;
		std::shared_ptr<dx12lib::IndexBuffer>  m_IndexBuffer  = nullptr;

		std::vector<VertexPosColor> VertexData;
		std::vector<uint16> IndicesData;

		friend class Renderer;
		friend class SceneRenderer;

	private:
	};
}