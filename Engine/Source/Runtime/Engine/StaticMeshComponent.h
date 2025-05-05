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

		inline AssetHandle<StaticMesh> GetMesh() { return Mesh; }
		void SetMesh(const AssetHandle<StaticMesh>& InHandle);

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		void SetMaterial(uint16 MaterialIndex, AssetHandle<Material>& InMaterial);

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		void ClearMesh();

		void UpdateMeshWithPath(const char* NewPath);
		void RefreshOverrideMaterials();

		bool IsUsingMaterial(const AssetHandle<Material>& Mat);
#endif

	protected:

		AssetHandle<StaticMesh> Mesh;
		StaticMeshSceneProxy* m_SceneProxy;

		std::vector<MaterialOverrideData> m_OverrideMaterials;

		friend class StaticMeshSceneProxy;

	private:

	};
}