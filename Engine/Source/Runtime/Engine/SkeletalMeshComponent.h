#pragma once

#include "PrimitiveComponent.h"
#include "ForwardTypes.h"

#include "Runtime/Core/AssetManager.h"
#include "Runtime/Engine/SkeletalMesh.h"

using namespace DirectX;

namespace Drn
{
	class SkeletalMeshSceneProxy;
	class MaterialInstanceDynamic;

	class SkeletalMeshComponent : public PrimitiveComponent
	{
	public:

		SkeletalMeshComponent();
		virtual ~SkeletalMeshComponent();

		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::SkeletalMeshComponent; }

		inline AssetHandle<SkeletalMesh> GetMesh() { return Mesh; }
		void SetMesh(const AssetHandle<SkeletalMesh>& InHandle);

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform(bool SkipPhysic) override;

		void SetMaterial(uint16 MaterialIndex, AssetHandle<Material>& InMaterial);
		void SetMaterial(uint16 MaterialIndex, AssetHandle<MaterialInstance>& InMaterial);
		void SetMaterial(uint16 MaterialIndex, TRefCountPtr<MaterialInstanceDynamic> InMaterial);

		inline uint16 GetMaterialCount() const { return m_OverrideMaterials.size(); };

		void RefreshOverrideMaterials();

		void SetMinDrawDistance(float Value);
		void SetMaxDrawDistance(float Value);

		//inline BodySetup* GetBodySetup() const { return Mesh.IsValid() ? Mesh->GetBodySetup() : nullptr; }
		virtual BoxSphereBounds CalcBounds(const Transform& LocalToWorld) const override;

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		void ClearMesh();

		void UpdateMeshWithPath(const char* NewPath);

		bool IsUsingMaterial(const AssetHandle<Material>& Mat);

		virtual void SetSelectedInEditor( bool SelectedInEditor, const HitProxyData& Data ) override;
		virtual void SetSelectable( bool Selectable ) override;

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
#endif

	protected:

		AssetHandle<SkeletalMesh> Mesh;
		SkeletalMeshSceneProxy* m_SkeletalMeshSceneProxy;

		std::vector<MaterialPropertyOverride> m_OverrideMaterials;

		float MinDrawDistance;
		float MaxDrawDistance;

		friend class SkeletalMeshSceneProxy;

	private:

	};
}