#pragma once

#include "ForwardTypes.h"

#define NUM_INSTANCED_CUSTOM_DATA 2

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

		virtual BoxSphereBounds CalcBounds(const Transform& LocalToWorld) const override;

		int32 AddInstance(const Transform& InstanceTransform);
		std::vector<int32> AddInstances(const std::vector<Transform>& InstanceTransforms, bool bShouldReturnIndices);
		int32 AddInstanceWorldSpace(const Transform& WorldTransform);

		bool UpdateInstanceTransform(int32 InstanceIndex, const Transform& NewInstanceTransform, bool bWorldSapce = false, bool bMarkRenderStateDirty = false, bool bTeleport = false);
		void SetCustomData(int32 DataIndex, int32 InstanceIndex, const Vector4& Value, bool bMarkRenderStateDirty = false);
		void SetCustomDataEnabled(int32 Index, bool bEnabled);

		bool GetInstanceTransform(int32 InstanceIndex, Transform& OutInstanceTransform, bool bWorldSpace = false) const;
		bool RemoveInstance(int32 InstanceIndex);
		void ClearInstances();

		inline int32 GetInstanceCount() const { return PerInstanceTransform.size(); }
		inline int32 GetInstanceRandomSeed() const { return ActualInstanceRandomSeed; }

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;
		void DrawInstances();

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

// ---------------------------------------------------------------------------------

		std::vector<Matrix> PerInstanceTransform;
		bool bCustomData[NUM_INSTANCED_CUSTOM_DATA];
		std::vector<Vector4> CustomData[NUM_INSTANCED_CUSTOM_DATA];

		int32 InstancingRandomSeed = 0; // if 0, generate at runtime
		int32 ActualInstanceRandomSeed;

		friend class InstancedStaticMeshSceneProxy;

	private:
	};
}