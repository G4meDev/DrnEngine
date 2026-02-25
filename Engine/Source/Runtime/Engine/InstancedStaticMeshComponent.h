#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class InstancedStaticMeshSceneProxy;
	class MaterialInstanceDynamic;

	class InstancedStaticMeshComponent: public PrimitiveComponent
	{
	public:
		InstancedStaticMeshComponent();
		virtual ~InstancedStaticMeshComponent();

		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::InstancedStaticMeshComponent; }

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

		BoxSphereBounds GetBounds();

		virtual BoxSphereBounds CalcBounds(const Transform& LocalToWorld) const override;

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		void ClearMesh();

		void UpdateMeshWithPath(const char* NewPath);

		bool IsUsingMaterial(const AssetHandle<Material>& Mat);

		virtual void SetSelectedInEditor( bool SelectedInEditor ) override;
		virtual void SetSelectable( bool Selectable ) override;

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
#endif

	protected:

		AssetHandle<StaticMesh> Mesh;
		InstancedStaticMeshSceneProxy* m_InstancedStaticMeshSceneProxy;

		std::vector<MaterialPropertyOverride> m_OverrideMaterials;

		float MinDrawDistance;
		float MaxDrawDistance;

		friend class InstancedStaticMeshSceneProxy;

	private:
	};
}