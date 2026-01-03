#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"
#include "Runtime/Renderer/MaterialPipelines.h"

LOG_DEFINE_CATEGORY( LogMaterial, "Material" );

namespace Drn
{
	Material::Material( const std::string& InPath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
		, m_SupportMainPass(true)
		, m_SupportPrePass(true)
		, m_SupportHitProxyPass(false)
		, m_SupportEditorPrimitivePass(false)
		, m_SupportEditorSelectionPass(true)
		, m_SupportDeferredDecalPass(false)
		, m_SupportStaticMeshDecalPass(false)
		, m_MaterialDomain(EMaterialDomain::Surface)
		, m_TwoSided(false)
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_RenderStateDirty(true)
		, m_SupportMainPass(true)
		, m_SupportPrePass(true)
		, m_SupportHitProxyPass(false)
		, m_SupportEditorPrimitivePass(false)
		, m_SupportEditorSelectionPass(true)
		, m_SupportDeferredDecalPass(false)
		, m_SupportStaticMeshDecalPass(false)
		, m_MaterialDomain(EMaterialDomain::Surface)
		, m_TwoSided(false)
	{
		m_SourcePath = InSourcePath;
		Import();
	}
#endif
	
	Material::~Material()
	{
		ReleasePSOs();
		
	}

	EAssetType Material::GetAssetType() { return EAssetType::Material; }

	void Material::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			ReleaseShaderBlobs();

			Ar >> m_SourcePath;

			m_MainShaderBlob.Serialize(Ar);
			m_HitProxyShaderBlob.Serialize(Ar);

			Ar >> *((uint8*)(&m_MaterialDomain));
			Ar >> m_TwoSided;

			uint8 Texture2DCount;
			Ar >> Texture2DCount;
			m_Texture2DSlots.clear();
			m_Texture2DSlots.reserve(Texture2DCount);
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots.push_back(MaterialIndexedTexture2DParameter());
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount;
			Ar >> TextureCubeCount;
			m_TextureCubeSlots.clear();
			m_TextureCubeSlots.reserve(TextureCubeCount);
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots.push_back(MaterialIndexedTextureCubeParameter());
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount;
			Ar >> ScalarCount;
			m_FloatSlots.clear();
			m_FloatSlots.reserve(ScalarCount);
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots.push_back(MaterialIndexedFloatParameter());
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count;
			Ar >> Vector4Count;
			m_Vector4Slots.clear();
			m_Vector4Slots.reserve(Vector4Count);
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots.push_back(MaterialIndexedVector4Parameter());
				m_Vector4Slots[i].Serialize(Ar);
			}

			Ar >> m_SupportHitProxyPass;
			Ar >> m_SupportMainPass;
			Ar >> m_SupportEditorPrimitivePass;

			m_EditorPrimitiveShaderBlob.Serialize(Ar);

			Ar >> m_SupportEditorSelectionPass;

			Ar >> m_SupportShadowPass;
			m_PointlightShadowDepthShaderBlob.Serialize(Ar);
			m_SpotlightShadowDepthShaderBlob.Serialize(Ar);

			Ar >> m_SupportDeferredDecalPass;
			m_DeferredDecalShaderBlob.Serialize(Ar);

			Ar >> m_SupportStaticMeshDecalPass;
			m_StaticMeshDecalShaderBlob.Serialize(Ar);

			Ar >> m_SupportPrePass;
			Ar >> m_HasCustomPrePass;
			m_PrePassShaderBlob.Serialize(Ar);
		}

		else
		{
			Ar << m_SourcePath; 
			
			m_MainShaderBlob.Serialize(Ar);
			m_HitProxyShaderBlob.Serialize(Ar);

			Ar << static_cast<uint8>(m_MaterialDomain);
			Ar << m_TwoSided;

			uint8 Texture2DCount = m_Texture2DSlots.size();
			Ar << Texture2DCount;
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount = m_TextureCubeSlots.size();
			Ar << TextureCubeCount;
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount = m_FloatSlots.size();
			Ar << ScalarCount;
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count = m_Vector4Slots.size();
			Ar << Vector4Count;
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots[i].Serialize(Ar);
			}

			Ar << m_SupportHitProxyPass;
			Ar << m_SupportMainPass;
			Ar << m_SupportEditorPrimitivePass;
			m_EditorPrimitiveShaderBlob.Serialize(Ar);
			Ar << m_SupportEditorSelectionPass;

			Ar << m_SupportShadowPass;
			m_PointlightShadowDepthShaderBlob.Serialize(Ar);
			m_SpotlightShadowDepthShaderBlob.Serialize(Ar);

			Ar << m_SupportDeferredDecalPass;
			m_DeferredDecalShaderBlob.Serialize(Ar);

			Ar << m_SupportStaticMeshDecalPass;
			m_StaticMeshDecalShaderBlob.Serialize(Ar);

			Ar << m_SupportPrePass;
			Ar << m_HasCustomPrePass;
			m_PrePassShaderBlob.Serialize(Ar);
		}
	}

	void Material::ReleaseShaderBlobs()
	{
		m_MainShaderBlob.ReleaseBlobs();
		m_PrePassShaderBlob.ReleaseBlobs();
		m_HitProxyShaderBlob.ReleaseBlobs();
		m_EditorPrimitiveShaderBlob.ReleaseBlobs();
		m_PointlightShadowDepthShaderBlob.ReleaseBlobs();
		m_SpotlightShadowDepthShaderBlob.ReleaseBlobs();
		m_DeferredDecalShaderBlob.ReleaseBlobs();
		m_StaticMeshDecalShaderBlob.ReleaseBlobs();
	}

	void Material::ReleasePSOs()
	{
		m_MaterialPipelines = nullptr;
	}

#if WITH_EDITOR
	void Material::Import()
	{
		//Renderer::Get()->Flush();
		AssetImporterMaterial::Import( this, m_SourcePath );
		Save();
		Load();

		MarkRenderStateDirty();
	}


	void Material::OpenAssetPreview()
	{
		if (!GuiLayer)
		{
			GuiLayer = new AssetPreviewMaterialGuiLayer( this );
			GuiLayer->Attach();
		}
	}

	void Material::CloseAssetPreview()
	{
		if ( GuiLayer )
		{
			GuiLayer->DeAttach();
			delete GuiLayer;
			GuiLayer = nullptr;
		}
	}
#endif

// ---------------------------------------------------------------------------------------------------------------

	void Material::UploadResources( D3D12CommandList* CommandList )
	{
		if (IsRenderStateDirty())
		{
			SCOPE_STAT();

			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			for (Texture2DProperty& Slot : m_Texture2DSlots)
			{
				if (!Slot.m_Texture2D.IsValid())
				{
					Slot.m_Texture2D.LoadChecked();
				}
			}

			for (TextureCubeProperty& Slot : m_TextureCubeSlots)
			{
				if (!Slot.m_TextureCube.IsValid())
				{
					Slot.m_TextureCube.LoadChecked();
				}
			}

			ReleasePSOs();

			m_MaterialPipelines = new MaterialPipelines(CommandList, this);

			ClearRenderStateDirty();
		}

		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			if ( m_Texture2DSlots[i].m_Texture2D.IsValid())
			{
				m_Texture2DSlots[i].m_Texture2D->UploadResources(CommandList);
			}
		}

		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			if (m_TextureCubeSlots[i].m_TextureCube.IsValid())
			{
				m_TextureCubeSlots[i].m_TextureCube->UploadResources(CommandList);
			}
		}

		const int32 Vector4SlotCount = m_Vector4Slots.size() * 4;
		const int32 ScalarSlotCount = m_FloatSlots.size();
		const int32 Texture2DSlotCount = m_Texture2DSlots.size() * 2;
		const int32 TextureCubeSlotCount = m_TextureCubeSlots.size() * 2;
		const int32 SlotCount = Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount + TextureCubeSlotCount;

		std::vector<uint32> Parameters;
		Parameters.resize(SlotCount);

		for (int i = 0; i < m_Vector4Slots.size(); i++)
		{
			*(Vector4*)(&Parameters[m_Vector4Slots[i].m_Index * 4]) = m_Vector4Slots[i].m_Value;
		}

		for (int i = 0; i < m_FloatSlots.size(); i++)
		{
			*(float*)(&Parameters[m_FloatSlots[i].m_Index + Vector4SlotCount]) = m_FloatSlots[i].m_Value;
		}

		for (int i = 0; i < m_Texture2DSlots.size(); i++)
		{
			Parameters[m_Texture2DSlots[i].m_Index * 2 + Vector4SlotCount + ScalarSlotCount] = m_Texture2DSlots[i].m_Texture2D.IsValid() ? m_Texture2DSlots[i].m_Texture2D->GetTextureIndex() : 0;
			Parameters[m_Texture2DSlots[i].m_Index * 2 + 1 + Vector4SlotCount + ScalarSlotCount] = m_Texture2DSlots[i].m_Texture2D.IsValid() ? m_Texture2DSlots[i].m_Texture2D->GetSamplerIndex() : 0;
		}

		for (int i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			Parameters[m_TextureCubeSlots[i].m_Index * 2 + Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount] = m_TextureCubeSlots[i].m_TextureCube.IsValid() ? m_TextureCubeSlots[i].m_TextureCube->GetTextureIndex() : 0;
			Parameters[m_TextureCubeSlots[i].m_Index * 2 + 1 + Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount] = m_TextureCubeSlots[i].m_TextureCube.IsValid() ? m_TextureCubeSlots[i].m_TextureCube->GetSamplerIndex() : 0;
		}

		if ( Parameters.size() > 0 )
		{
			//uint32 Size = Align(TextureIndices.size() * sizeof(uint32), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			uint32 Size = Parameters.size() * sizeof(uint32);
			ParametersBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), Size, EUniformBufferUsage::SingleFrame, Parameters.data());
		}
	}

	void Material::BindMainPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_MainPassPSO);

		BindResources(CommandList);
	}

	void Material::BindPrePass( D3D12CommandList* CommandList )
	{
		TRefCountPtr<GraphicsPipelineState> PSO;
		if (m_HasCustomPrePass)
		{
			PSO = m_MaterialPipelines->m_PrePassPSO;
		}
		
		else
		{
			PSO = m_TwoSided 
				? CommonResources::Get()->m_PositionOnlyDepthPSO->m_CullNonePSO
				: CommonResources::Get()->m_PositionOnlyDepthPSO->m_CullBackPSO;
		}

		if (!PSO)
		{
			return;
		}

		CommandList->SetGraphicPipelineState(PSO);

		BindResources(CommandList);
	}

	void Material::BindPointLightShadowDepthPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_PointLightShadowDepthPassPSO);
		
		BindResources(CommandList);
	}

	void Material::BindSpotLightShadowDepthPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_SpotLightShadowDepthPassPSO);
		
		BindResources(CommandList);
	}

	void Material::BindDeferredDecalPass( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_DeferredDecalPassPSO);

		BindResources(CommandList);
	}

	void Material::BindStaticMeshDecalPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_StaticMeshDecalPassPSO);

		BindResources(CommandList);
	}

#if WITH_EDITOR
	void Material::BindEditorPrimitivePass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_EditorProxyPSO);

		BindResources(CommandList);
	}

	void Material::BindSelectionPass( D3D12CommandList* CommandList )
	{
		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_SelectionPassPSO);
		CommandList->GetD3D12CommandList()->OMSetStencilRef( 255 );

		BindResources(CommandList);
	}

	void Material::BindHitProxyPass( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicPipelineState(m_MaterialPipelines->m_HitProxyPassPSO);

		BindResources(CommandList);
	}

#endif

	void Material::BindResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicRootConstant(ParametersBuffer ? ParametersBuffer->GetViewIndex() : 0, 3);
	}

	void Material::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			const Texture2DProperty& TextureSlot = m_Texture2DSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTexture2D(i, TextureAsset);
				return;
			}
		}
	}

	void Material::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_Texture2DSlots.size())
		{
			m_Texture2DSlots[Index].m_Texture2D = TextureAsset;
		}
	}

	void Material::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			const TextureCubeProperty& TextureSlot = m_TextureCubeSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTextureCube(i, TextureAsset);
				return;
			}
		}
	}

	void Material::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_TextureCubeSlots.size())
		{
			m_TextureCubeSlots[Index].m_TextureCube = TextureAsset;
		}
	}

	void Material::SetIndexedScalar( uint32 Index, float Value )
	{
		if (Index >= 0 && Index < m_FloatSlots.size())
		{
			m_FloatSlots[Index].m_Value = Value;
		}
	}

	void Material::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		if (Index >= 0 && Index < m_Vector4Slots.size())
		{
			m_Vector4Slots[Index].m_Value = Value;
		}
	}

	void Material::SetNamedScalar( const std::string& Name, float Value )
	{
		for ( int32 i = 0; i < m_FloatSlots.size(); i++)
		{
			if (m_FloatSlots[i].m_Name == Name)
			{
				SetIndexedScalar(i, Value);
			}
		}
	}

	void Material::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		for ( int32 i = 0; i < m_Vector4Slots.size(); i++)
		{
			if (m_Vector4Slots[i].m_Name == Name)
			{
				SetIndexedVector(i, Value);
			}
		}
	}

        }