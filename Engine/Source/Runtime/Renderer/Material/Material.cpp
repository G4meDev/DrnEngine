#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"

LOG_DEFINE_CATEGORY( LogMaterial, "Material" );

namespace Drn
{
	Material::Material( const std::string& InPath )
		: Asset(InPath)
		, m_MainPassPSO(nullptr)
		, m_PrePassPSO(nullptr)
		, m_PointLightShadowDepthPassPSO(nullptr)
		, m_SpotLightShadowDepthPassPSO(nullptr)
		, m_DeferredDecalPassPSO(nullptr)
		, m_StaticMeshDecalPassPSO(nullptr)
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
		, m_TextureIndexBuffer(nullptr)
		, m_ScalarBuffer(nullptr)
		, m_VectorBuffer(nullptr)
		, m_ScalarBufferDirty(true)
		, m_VectorBufferDirty(true)
		, m_TextureBufferDirty(true)
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_MainPassPSO(nullptr)
		, m_PrePassPSO(nullptr)
		, m_PointLightShadowDepthPassPSO(nullptr)
		, m_SpotLightShadowDepthPassPSO(nullptr)
		, m_SelectionPassPSO(nullptr)
		, m_HitProxyPassPSO(nullptr)
		, m_EditorProxyPSO(nullptr)
		, m_DeferredDecalPassPSO(nullptr)
		, m_StaticMeshDecalPassPSO(nullptr)
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
		, m_TextureIndexBuffer(nullptr)
		, m_ScalarBuffer(nullptr)
		, m_VectorBuffer(nullptr)
		, m_ScalarBufferDirty(true)
		, m_VectorBufferDirty(true)
		, m_TextureBufferDirty(true)
	{
		m_SourcePath = InSourcePath;
		Import();
	}
#endif
	
	Material::~Material()
	{
		ReleasePSOs();

		ReleaseBuffers();

		
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
		if (m_MainPassPSO)
		{
			m_MainPassPSO->ReleaseBufferedResource();
			m_MainPassPSO = nullptr;
		}

		if (m_PrePassPSO)
		{
			m_PrePassPSO->ReleaseBufferedResource();
			m_PrePassPSO = nullptr;
		}

		if ( m_PointLightShadowDepthPassPSO )
		{
			m_PointLightShadowDepthPassPSO->ReleaseBufferedResource();
			m_PointLightShadowDepthPassPSO = nullptr;
		}

		if ( m_SpotLightShadowDepthPassPSO )
		{
			m_SpotLightShadowDepthPassPSO->ReleaseBufferedResource();
			m_SpotLightShadowDepthPassPSO = nullptr;
		}

		if ( m_DeferredDecalPassPSO )
		{
			m_DeferredDecalPassPSO->ReleaseBufferedResource();
			m_DeferredDecalPassPSO = nullptr;
		}

		if ( m_StaticMeshDecalPassPSO )
		{
			m_StaticMeshDecalPassPSO->ReleaseBufferedResource();
			m_StaticMeshDecalPassPSO = nullptr;
		}

#if WITH_EDITOR
		if (m_SelectionPassPSO)
		{
			m_SelectionPassPSO->ReleaseBufferedResource();
			m_SelectionPassPSO = nullptr;
		}

		if (m_HitProxyPassPSO)
		{
			m_HitProxyPassPSO->ReleaseBufferedResource();
			m_HitProxyPassPSO = nullptr;
		}

		if (m_EditorProxyPSO)
		{
			m_EditorProxyPSO->ReleaseBufferedResource();
			m_HitProxyPassPSO = nullptr;
		}
#endif
	}

	void Material::ReleaseBuffers()
	{
		if (m_TextureIndexBuffer)
		{
			m_TextureIndexBuffer->ReleaseBufferedResource();
			m_TextureIndexBuffer = nullptr;
		}

		if (m_ScalarBuffer)
		{
			m_ScalarBuffer->ReleaseBufferedResource();
			m_ScalarBuffer = nullptr;
		}

		if (m_VectorBuffer)
		{
			m_VectorBuffer->ReleaseBufferedResource();
			m_VectorBuffer = nullptr;
		}

	}

	void Material::InitalizeParameterMap()
	{
		m_ScalarMap.clear();
		m_Vector4Map.clear();

		for ( int i = 0; i < m_FloatSlots.size(); i++ )
		{
			m_ScalarMap[m_FloatSlots[i].m_Name] = &m_FloatSlots[i];
		}

		for ( int i = 0; i < m_Vector4Slots.size(); i++ )
		{
			m_Vector4Map[m_Vector4Slots[i].m_Name] = &m_Vector4Slots[i];
		}
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

	void Material::UploadResources( ID3D12GraphicsCommandList2* CommandList )
	{
		if (IsRenderStateDirty())
		{
			SCOPE_STAT();

			InitalizeParameterMap();

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
			ReleaseBuffers();

#if D3D12_Debug_INFO
			std::string name = Path::ConvertShortPath(m_Path);
			name = Path::RemoveFileExtension(name);
#endif

			// TODO: support no constant buffers

			{
				m_TextureIndexBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
				m_TextureIndexBuffer->SetName("TextureBuffer_" + name);
#endif

				D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
				ResourceViewDesc.BufferLocation = m_TextureIndexBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
				ResourceViewDesc.SizeInBytes = 256;
				Device->CreateConstantBufferView( &ResourceViewDesc, m_TextureIndexBuffer->GetCpuHandle());

			}

			{
				const size_t ScalarBufferSize = m_FloatSlots.size() * sizeof( float );
				//const size_t ScalarBufferSizePadded = ( ScalarBufferSize + 255 ) & ~255 + 256;
				// TODO: remove
				const size_t ScalarBufferSizePadded = 256;

				m_ScalarBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( ScalarBufferSizePadded ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
				m_ScalarBuffer->SetName("ScalarBuffer_" + name);
#endif

				D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
				ResourceViewDesc.BufferLocation = m_ScalarBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
				ResourceViewDesc.SizeInBytes = ScalarBufferSizePadded;
				Device->CreateConstantBufferView( &ResourceViewDesc , m_ScalarBuffer->GetCpuHandle());

			}

			{
				const size_t VectorBufferSize = m_Vector4Slots.size() * sizeof( Vector4 );
				//const size_t VectorBufferSizePadded = ( VectorBufferSize + 255 ) & ~255;
				// TODO: remove
				const size_t VectorBufferSizePadded = 256;

				m_VectorBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( VectorBufferSizePadded ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
				m_VectorBuffer->SetName("VectorBuffer_" + name);
#endif

				D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
				ResourceViewDesc.BufferLocation = m_VectorBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
				ResourceViewDesc.SizeInBytes = VectorBufferSizePadded;
				Device->CreateConstantBufferView( &ResourceViewDesc , m_VectorBuffer->GetCpuHandle());

			}

			const D3D12_CULL_MODE CullMode = GetCullMode();

			if (m_SupportMainPass)
			{
				m_MainPassPSO = PipelineStateObject::CreateMainPassPSO(CullMode, EInputLayoutType::StandardMesh,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_MainShaderBlob);

#if D3D12_Debug_INFO
				m_MainPassPSO->SetName( "PSO_MainPass_" + name );
#endif
			}

			if (m_SupportPrePass && m_HasCustomPrePass)
			{
				m_PrePassPSO = PipelineStateObject::CreatePrePassPSO(CullMode, EInputLayoutType::StandardMesh, m_PrePassShaderBlob);

#if D3D12_Debug_INFO
				m_PrePassPSO->SetName( "PSO_PrePass_" + name );
#endif
			}

			if (m_SupportDeferredDecalPass)
			{
				m_DeferredDecalPassPSO = PipelineStateObject::CreateDecalPassPSO(m_DeferredDecalShaderBlob);

#if D3D12_Debug_INFO
				m_DeferredDecalPassPSO->SetName( "PSO_DecalPass_" + name );
#endif
			}

			if (m_SupportStaticMeshDecalPass)
			{
				m_StaticMeshDecalPassPSO = PipelineStateObject::CreateMeshDecalPassPSO( GetCullMode(), m_StaticMeshDecalShaderBlob);

#if D3D12_Debug_INFO
				m_StaticMeshDecalPassPSO->SetName( "PSO_StaticMeshDecalPass_" + name );
#endif
			}

			if (m_SupportShadowPass)
			{
				m_PointLightShadowDepthPassPSO = PipelineStateObject::CreatePointLightShadowDepthPassPSO(
					CullMode, EInputLayoutType::StandardMesh,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_PointlightShadowDepthShaderBlob);

				m_SpotLightShadowDepthPassPSO = PipelineStateObject::CreateSpotLightShadowDepthPassPSO(
					CullMode, EInputLayoutType::StandardMesh,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_SpotlightShadowDepthShaderBlob);

#if D3D12_Debug_INFO
				m_PointLightShadowDepthPassPSO->SetName( "PSO_Po intLightShadowDepthPass_" + name );
				m_SpotLightShadowDepthPassPSO->SetName( "PSO_SpotLightShadowDepthPass_" + name );
#endif
			}

#if WITH_EDITOR

			if (m_SupportEditorSelectionPass)
			{
				m_SelectionPassPSO = PipelineStateObject::CreateSelectionPassPSO(CullMode, EInputLayoutType::StandardMesh,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_MainShaderBlob);
#if D3D12_Debug_INFO
				m_SelectionPassPSO->SetName( "PSO_SelectionPass_" + name );
#endif
			}

			if (IsSupportingHitProxyPass())
			{
				m_HitProxyPassPSO = PipelineStateObject::CreateHitProxyPassPSO(CullMode, EInputLayoutType::StandardMesh,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_HitProxyShaderBlob);

#if D3D12_Debug_INFO
				m_HitProxyPassPSO->SetName( "PSO_HitProxyPass_" + name );
#endif
			}

			if (m_SupportEditorPrimitivePass)
			{
				m_EditorProxyPSO = PipelineStateObject::CreateEditorPrimitivePassPSO(CullMode, EInputLayoutType::StandardMesh,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, m_EditorPrimitiveShaderBlob);
#if D3D12_Debug_INFO
				m_EditorProxyPSO->SetName( "PSO_EditorPrimitive_" + name );
#endif
			}

#endif

			ClearRenderStateDirty();
		}

		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			m_Texture2DSlots[i].m_Texture2D->UploadResources(CommandList);
		}

		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			if (m_TextureCubeSlots[i].m_TextureCube.IsValid())
			{
				m_TextureCubeSlots[i].m_TextureCube->UploadResources(CommandList);
			}
		}
	}

	void Material::BindMainPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_MainPassPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

	void Material::BindPrePass( ID3D12GraphicsCommandList2* CommandList )
	{
		ID3D12PipelineState* PSO;
		if (m_HasCustomPrePass)
		{
			PSO = m_PrePassPSO->GetD3D12PSO();
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

		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(PSO);

		BindResources(CommandList);
	}

	void Material::BindPointLightShadowDepthPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_PointLightShadowDepthPassPSO->GetD3D12PSO());
		
		BindResources(CommandList);
	}

	void Material::BindSpotLightShadowDepthPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_SpotLightShadowDepthPassPSO->GetD3D12PSO());
		
		BindResources(CommandList);
	}

	void Material::BindDeferredDecalPass( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_DeferredDecalPassPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

	void Material::BindStaticMeshDecalPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_StaticMeshDecalPassPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

#if WITH_EDITOR
	void Material::BindEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_EditorProxyPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

	void Material::BindSelectionPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_SelectionPassPSO->GetD3D12PSO());
		CommandList->OMSetStencilRef( 255 );

		BindResources(CommandList);
	}

	void Material::BindHitProxyPass( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT();

		CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
		CommandList->SetPipelineState(m_HitProxyPassPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

#endif

	void Material::BindResources( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT();

		if (m_TextureBufferDirty)
		{
			m_TextureBufferDirty = false;

			std::vector<uint32> TextureIndices;
			TextureIndices.resize(m_Texture2DSlots.size() + m_TextureCubeSlots.size());

			for (auto& TextureSlot : m_Texture2DSlots)
			{
				TextureIndices[TextureSlot.m_Index] = TextureSlot.m_Texture2D.IsValid() ? TextureSlot.m_Texture2D->GetTextureIndex() : 0;
			}

			for (auto& TextureSlot : m_TextureCubeSlots)
			{
				TextureIndices[TextureSlot.m_Index] = TextureSlot.m_TextureCube.IsValid() ? TextureSlot.m_TextureCube->GetTextureIndex() : 0;
			}

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_TextureIndexBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, TextureIndices.data(), TextureIndices.size() * sizeof(uint32));
			m_TextureIndexBuffer->GetD3D12Resource()->Unmap(0, nullptr);
		}
		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_TextureIndexBuffer->GetGpuHandle()), 3);

		if (m_ScalarBufferDirty)
		{
			m_ScalarBufferDirty = false;

			std::vector<float> Values;
			for (int i = 0; i < m_FloatSlots.size(); i++)
			{
				Values.push_back(m_FloatSlots[i].m_Value);
			}

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ScalarBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, Values.data(), Values.size() * sizeof(float) );
			m_ScalarBuffer->GetD3D12Resource()->Unmap(0, nullptr);
		}
		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_ScalarBuffer->GetGpuHandle()), 4);

		if (m_VectorBufferDirty)
		{
			m_VectorBufferDirty = false;

			std::vector<Vector4> Values;
			for (int i = 0; i < m_Vector4Slots.size(); i++)
			{
				Values.push_back(m_Vector4Slots[i].m_Value);
			}

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_VectorBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, Values.data(), Values.size() * sizeof(Vector4) );
			m_VectorBuffer->GetD3D12Resource()->Unmap(0, nullptr);
		}
		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_VectorBuffer->GetGpuHandle()), 5);
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
			m_TextureBufferDirty = true;
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
			m_TextureBufferDirty = true;
		}
	}

	void Material::SetIndexedScalar( uint32 Index, float Value )
	{
		if (Index >= 0 && Index < m_FloatSlots.size())
		{
			m_FloatSlots[Index].m_Value = Value;
			m_ScalarBufferDirty = true;
		}
	}

	void Material::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		if (Index >= 0 && Index < m_Vector4Slots.size())
		{
			m_Vector4Slots[Index].m_Value = Value;
			m_VectorBufferDirty = true;
		}
	}

	void Material::SetNamedScalar( const std::string& Name, float Value )
	{
		auto it = m_ScalarMap.find(Name);

		if ( it != m_ScalarMap.end() )
		{
			SetIndexedScalar(it->second->m_Index, Value);
		}
	}

	void Material::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		auto it = m_Vector4Map.find(Name);
		
		if ( it != m_Vector4Map.end() )
		{
			SetIndexedVector(it->second->m_Index, Value);
		}
	}

}