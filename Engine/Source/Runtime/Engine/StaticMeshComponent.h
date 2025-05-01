#pragma once

#include "PrimitiveComponent.h"
#include "ForwardTypes.h"

#include "Runtime/Core/AssetManager.h"
#include "Runtime/Engine/StaticMesh.h"

using namespace DirectX;

namespace Drn
{
	class StaticMeshSceneProxy;

	class StaticMeshComponent : public PrimitiveComponent
	{
	public:

		StaticMeshComponent();
		virtual ~StaticMeshComponent();

		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::StaticMeshComponent; }

		inline StaticMesh* GetMesh() { return Mesh.Get(); }
		void SetMesh(const AssetHandle<StaticMesh>& InHandle);

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		void ClearMesh();

		void UpdateMeshWithPath(const char* NewPath);

#endif

	protected:

		AssetHandle<StaticMesh> Mesh;
		StaticMeshSceneProxy* m_SceneProxy;


	private:
	};
}