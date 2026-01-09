#pragma once

#include "PrimitiveComponent.h"
#include "ForwardTypes.h"

#include "Runtime/Core/AssetManager.h"
#include "Runtime/Engine/StaticMesh.h"

using namespace DirectX;

namespace Drn
{
	class StaticMeshSceneProxy;
	class MaterialInstanceDynamic;

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
		void SetMaterial(uint16 MaterialIndex, AssetHandle<MaterialInstance>& InMaterial);
		void SetMaterial(uint16 MaterialIndex, TRefCountPtr<MaterialInstanceDynamic> InMaterial);

		void RefreshOverrideMaterials();

		void SetMinDrawDistance(float Value);
		void SetMaxDrawDistance(float Value);

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		void ClearMesh();

		void UpdateMeshWithPath(const char* NewPath);

		bool IsUsingMaterial(const AssetHandle<Material>& Mat);

		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
		virtual void SetSelectable( bool Selectable ) override;
#endif

	protected:

		AssetHandle<StaticMesh> Mesh;
		StaticMeshSceneProxy* m_SceneProxy;

		std::vector<MaterialPropertyOverride> m_OverrideMaterials;

		float MinDrawDistance;
		float MaxDrawDistance;

		friend class StaticMeshSceneProxy;

	private:

	};
}