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

		StaticMeshComponent();
		virtual ~StaticMeshComponent();

		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::StaticMeshComponent; }

		inline StaticMesh* GetMesh() { return Mesh.Get(); }
		void SetMesh(const AssetHandle<StaticMesh>& InHandle);

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		void ClearMesh();

#endif

	protected:

		AssetHandle<StaticMesh> Mesh;


	private:
	};
}