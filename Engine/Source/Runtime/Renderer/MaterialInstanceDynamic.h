#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/MaterialShared.h"
#include "Runtime/Renderer/MaterialInterface.h"

namespace Drn
{
	class MaterialInstanceDynamic : public MaterialInterface, public IRefCountedObject
	{
	public:
		virtual ~MaterialInstanceDynamic();
		static TRefCountPtr<MaterialInstanceDynamic> Create(AssetHandle<Material> InParent);

		uint32 AddRef() const override { return ++NumRefs; }
		uint32 Release() const override
		{
			uint32 Refs = --NumRefs;
			if(Refs == 0)
			{
				delete this;
			}
			return Refs;
		}

		uint32 GetRefCount() const override { return NumRefs; }

		inline bool IsRenderStateDirty() const { return m_RenderStateDirty; }
		inline void MarkRenderStateDirty() { m_RenderStateDirty = true; }
		inline void ClearRenderStateDirty() { m_RenderStateDirty = false; }

// -------------------------------------------------------------------------------------------------------------
// ---------------------------------------- Material Interface -------------------------------------------------
// -------------------------------------------------------------------------------------------------------------

		virtual Material* GetMaterial() const override { return *Parent; };
		virtual bool IsDependent(MaterialInterface* OtherMaterial) const override;

		virtual void UploadResources( class D3D12CommandList* CommandList ) override;
		virtual void BindResources( D3D12CommandList* CommandList ) override;

		virtual void SetNamedTexture2D(const std::string& Name, AssetHandle<Texture2D> TextureAsset) override;
		virtual void SetIndexedTexture2D(uint8 Index, AssetHandle<Texture2D> TextureAsset) override;

		virtual void SetNamedTextureCube(const std::string& Name, AssetHandle<TextureCube> TextureAsset) override;
		virtual void SetIndexedTextureCube(uint8 Index, AssetHandle<TextureCube> TextureAsset) override;

		virtual void SetIndexedScalar(uint32 Index, float Value) override;
		virtual void SetIndexedVector(uint32 Index, const Vector4& Value) override;
		
		virtual void SetNamedScalar(const std::string& Name, float Value) override;
		virtual void SetNamedVector4(const std::string& Name, const Vector4& Value) override;

	private:
		MaterialInstanceDynamic();

		AssetHandle<Material> Parent;
		MaterialUniformParameters MaterialParameters;
		bool m_RenderStateDirty;

		mutable uint32 NumRefs;
	};
}