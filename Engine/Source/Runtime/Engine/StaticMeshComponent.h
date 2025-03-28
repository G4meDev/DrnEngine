#pragma once

#include "PrimitiveComponent.h"
#include "ForwardTypes.h"

#include "Runtime/Core/AssetManager.h"
#include "Runtime/Engine/StaticMesh.h"

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

		inline StaticMesh* GetMesh() { return Mesh.Get(); }
		void SetMesh(const AssetHandle<StaticMesh>& InHandle);

	protected:

		AssetHandle<StaticMesh> Mesh;


	private:
	};
}