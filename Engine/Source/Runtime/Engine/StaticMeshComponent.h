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

		StaticMesh* Mesh;

	protected:

		std::string MeshPath;



	private:
	};
}