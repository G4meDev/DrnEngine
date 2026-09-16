#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class DynamicMeshSceneProxy;

	struct DynamicMeshSection
	{
		DynamicMeshSection()
			: bEnableCollision(false)
			, bSectionVisible(true)
			, bSectionDirty(true)
		{
			SectionLocalBox.Init();
		}

		void Reset()
		{
			VertexBufferData;

			SectionLocalBox.Init();
			bEnableCollision = false;
			bSectionVisible = true;
			bSectionDirty = true;
		}

		StaticMeshVertexData VertexBufferData;
		MaterialSlot SectionMaterial;
		Box SectionLocalBox;
		bool bEnableCollision;
		bool bSectionVisible;
		bool bSectionDirty;

		friend class Archive& operator<<(Archive& Ar, const DynamicMeshSection& Value);
		friend class Archive& operator>>(Archive& Ar, DynamicMeshSection& Value);
	};

	class DynamicMeshComponent : public PrimitiveComponent
	{
	public:

		DynamicMeshComponent();
		virtual ~DynamicMeshComponent();

		virtual void Tick(float DeltaTime) override;

		inline virtual EComponentType GetComponentType() override { return EComponentType::DynamicMeshComponent; }

		virtual void Serialize( Archive& Ar ) override;

		virtual void RegisterComponent(World* InOwningWorld) override;
		virtual void UnRegisterComponent() override;

		virtual void OnUpdateTransform(bool SkipPhysic) override;

		void CreatePhysicState();
		void DestroyPhysicState();
		void RecreatePhysicState();

		void SetMaterial(uint16 MaterialIndex, AssetHandle<Material>& InMaterial);
		void SetMaterial(uint16 MaterialIndex, AssetHandle<MaterialInstance>& InMaterial);
		void SetMaterial(uint16 MaterialIndex, TRefCountPtr<MaterialInstanceDynamic> InMaterial);

		void SetMinDrawDistance(float Value);
		void SetMaxDrawDistance(float Value);

		//inline BodySetup* GetBodySetup() const { return Mesh.IsValid() ? Mesh->GetBodySetup() : nullptr; }
		virtual BoxSphereBounds CalcBounds(const Transform& LocalToWorld) const override;

		void CreateMeshSection_Color(int32 SectionIndex, MaterialSlot InMaterial, const std::vector<Vector>& Positions, const std::vector<uint32>& Indices, const std::vector<Color>& Colors);

		void CreateMeshSection(int32 SectionIndex, MaterialSlot InMaterial, const std::vector<Vector>& Positions, const std::vector<uint32>& Indices, const std::vector<Vector>& Normals, const std::vector<Vector>& Tangents,
			const std::vector<Color>& Colors, const std::vector<Vector2>& UV0, const std::vector<Vector2>& UV1, const std::vector<Vector2>& UV2, const std::vector<Vector2>& UV3);

		void ClearSections();

		void CalculateLocalBounds();

#if WITH_EDITOR

		virtual void DrawDetailPanel(float DeltaTime) override;

		bool IsUsingMaterial(const AssetHandle<Material>& Mat);

		virtual void SetSelectedInEditor( bool SelectedInEditor, const HitProxyData& Data ) override;
		virtual void SetSelectable( bool Selectable ) override;

		virtual void DrawEditorDefault() override;
		virtual void DrawEditorSelected() override;
#endif

	protected:

		DynamicMeshSceneProxy* m_DynamicMeshSceneProxy;

		std::vector<DynamicMeshSection> MeshSecions;

		float MinDrawDistance;
		float MaxDrawDistance;

		BoxSphereBounds LocalBounds;

		friend class DynamicMeshSceneProxy;

	private:

	};
}