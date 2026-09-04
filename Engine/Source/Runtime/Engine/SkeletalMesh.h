#pragma once

#include "ForwardTypes.h"
#include "Runtime/Engine/SkeletalMeshVertexData.h"

namespace Drn
{
	struct MeshBoneInfo
	{
		std::string Name; // @TODO: cached hashed string
		int32 ParentIndex;

		MeshBoneInfo(const std::string& InName, int32 InParentIndex)
			: Name(InName)
			, ParentIndex(InParentIndex)
		{}

		MeshBoneInfo() : MeshBoneInfo("Invalid", -1)
		{}

		bool operator==( const MeshBoneInfo& Other ) const { return Name == Other.Name; }

		friend Archive& operator<<(Archive& Ar, const MeshBoneInfo& Value);
		friend Archive& operator>>(Archive& Ar, MeshBoneInfo& Value);
	};

	struct ReferenceSkeleton
	{
		std::vector<MeshBoneInfo> BoneInfo;
		std::vector<Transform> BonePose;

		inline void Resize(int32 NewSize) { BoneInfo.resize(NewSize); BonePose.resize(NewSize); }

		bool HasBone(const std::string& Name) const;
		bool IsLeafBone(int32 BoneIndex) const;
		int32 FindBone(const std::string& Name) const;

		friend Archive& operator<<(Archive& Ar, const ReferenceSkeleton& Value);
		friend Archive& operator>>(Archive& Ar, ReferenceSkeleton& Value);
	};

	struct SkeletalMeshSlotData : public Serializable
	{
	public:
		SkeletalMeshSlotData();
		~SkeletalMeshSlotData();

		uint8 MaterialIndex = 0;

		SkeletalMeshVertexData VertexData;

		class SkeletalMeshVertexBuffer* m_SkeletalMeshVertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;

		virtual void Serialize(Archive& Ar) override;

		inline void ReleaseBuffers();

		void BindAndDraw( class D3D12CommandList* CommandList ) const;
	};

	struct SkeletalMeshData : public Serializable
	{
	public:
		SkeletalMeshData() {};

		std::vector<SkeletalMeshSlotData> MeshesData;
		std::vector<MaterialProperty> Materials;
		ReferenceSkeleton RefSkeleton;

		virtual void Serialize( Archive& Ar ) override;
	};

	class SkeletalMesh : public Asset
	{
	public:
		SkeletalMesh(const std::string& InPath);
		virtual ~SkeletalMesh();

#if WITH_EDITOR
		SkeletalMesh(const std::string& InPath, const std::string& InSourcePath);
#endif

		virtual void Serialize(Archive& Ar) override;

		void InitResources( ID3D12GraphicsCommandList2* CommandList );
		void UploadResources( class D3D12CommandList* CommandList );

		virtual EAssetType GetAssetType() override;
		inline static EAssetType GetAssetTypeStatic() { return EAssetType::SkeletalMesh; }

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

		inline const BoxSphereBounds& GetBounds() const { return Bounds; }
		inline BoxSphereBounds GetBounds() { return Bounds; }

		MaterialSlot GetMaterialAtIndex(uint32 Index);

		inline const SkeletalMeshData& GetData() const { return Data; }

	protected:

#if WITH_EDITOR
		void Import();
#endif

		bool m_RenderStateDirty;

		friend class Renderer;
		friend class SceneRenderer;

	private:

		std::string m_SourcePath;
		SkeletalMeshData Data;

		BoxSphereBounds Bounds;

		float ImportScale = 1.0f;

		bool m_ImportNormals = true;
		bool m_ImportTangents = true;
		bool m_ImportColor = false;
		uint8 m_ImportUVs = 1;

		Vector PositiveBoundExtention = Vector::ZeroVector;
		Vector NegativeBoundExtention = Vector::ZeroVector;

#if WITH_EDITOR
		virtual void OpenAssetPreview() override;
		virtual void CloseAssetPreview() override;

		class AssetPreviewSkeletalMeshGuiLayer* GuiLayer = nullptr;
#endif

		friend class SkeletalMeshComponent;
		friend class SkeletalMeshSceneProxy;
		friend class AssetImporterSkeletalMesh;
		friend class AssetPreviewSkeletalMeshGuiLayer;
	};
}