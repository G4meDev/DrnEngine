#include "DrnPCH.h"
#include "MaterialInstanceDynamic.h"

namespace Drn
{
	MaterialInstanceDynamic::MaterialInstanceDynamic()
		: m_RenderStateDirty(true)
		, NumRefs(0)
	{
		
	}

	MaterialInstanceDynamic::~MaterialInstanceDynamic()
	{
		
	}

	TRefCountPtr<MaterialInstanceDynamic> MaterialInstanceDynamic::Create( AssetHandle<Material> InParent )
	{
		drn_check(InParent.IsValid());

		MaterialInstanceDynamic* Result = new MaterialInstanceDynamic;
		Result->Parent = InParent;
		Result->MaterialParameters.CopyParameters(InParent->GetParameters());

		return Result;
	}

	bool MaterialInstanceDynamic::IsDependent( MaterialInterface* OtherMaterial ) const
	{
		return this == OtherMaterial || (Parent.Get() && Parent.Get() == OtherMaterial->GetMaterial());
	}

	void MaterialInstanceDynamic::UploadResources( class D3D12CommandList* CommandList )
	{
		// TODO: just need to upload pipeline states of parent
		Parent->UploadResources(CommandList);

		if (IsRenderStateDirty())
		{
			SCOPE_STAT();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			for (Texture2DProperty& Slot : MaterialParameters.m_Texture2DSlots)
			{
				if (!Slot.m_Texture2D.IsValid())
				{
					Slot.m_Texture2D.LoadChecked();
				}
			}
			
			for (TextureCubeProperty& Slot : MaterialParameters.m_TextureCubeSlots)
			{
				if (!Slot.m_TextureCube.IsValid())
				{
					Slot.m_TextureCube.LoadChecked();
				}
			}

			ClearRenderStateDirty();
		}

		MaterialParameters.UploadResources(CommandList);
	}

	void MaterialInstanceDynamic::BindResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicRootConstant(MaterialParameters.ParametersBuffer ? MaterialParameters.ParametersBuffer->GetViewIndex() : 0, 3);
	}

	void MaterialInstanceDynamic::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		MaterialParameters.SetNamedTexture2D(Name, TextureAsset);
	}

	void MaterialInstanceDynamic::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		MaterialParameters.SetIndexedTexture2D(Index, TextureAsset);
	}

	void MaterialInstanceDynamic::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		MaterialParameters.SetNamedTextureCube(Name, TextureAsset);
	}

	void MaterialInstanceDynamic::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		MaterialParameters.SetIndexedTextureCube(Index, TextureAsset);
	}

	void MaterialInstanceDynamic::SetIndexedScalar( uint32 Index, float Value )
	{
		MaterialParameters.SetIndexedScalar(Index, Value);
	}

	void MaterialInstanceDynamic::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		MaterialParameters.SetIndexedVector(Index, Value);
	}

	void MaterialInstanceDynamic::SetNamedScalar( const std::string& Name, float Value )
	{
		MaterialParameters.SetNamedScalar(Name, Value);
	}

	void MaterialInstanceDynamic::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		MaterialParameters.SetNamedVector4(Name, Value);
	}


}  // namespace Drn