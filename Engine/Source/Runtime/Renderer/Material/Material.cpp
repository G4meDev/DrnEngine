#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"

LOG_DEFINE_CATEGORY( LogMaterial, "Material" );

namespace Drn
{
	Material::Material( const std::string& InPath )
		: Asset(InPath)
		, m_ScalarCBV(nullptr)
		, m_RootSignature(nullptr)
		, m_MainPassPSO(nullptr)
		, m_RenderStateDirty(true)
		, m_SupportMainPass(true)
		, m_SupportHitProxyPass(false)
		, m_SupportEditorPrimitivePass(false)
		, m_SupportEditorSelectionPass(true)
		, m_PrimitiveType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		, m_InputLayoutType(EInputLayoutType::StandardMesh)
		, m_CullMode(D3D12_CULL_MODE_BACK)
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_ScalarCBV(nullptr)
		, m_RootSignature(nullptr)
		, m_MainPassPSO(nullptr)
		, m_SelectionPassPSO(nullptr)
		, m_HitProxyPassPSO(nullptr)
		, m_EditorProxyPSO(nullptr)
		, m_RenderStateDirty(true)
		, m_SupportMainPass(true)
		, m_SupportHitProxyPass(false)
		, m_SupportEditorPrimitivePass(false)
		, m_SupportEditorSelectionPass(true)
		, m_PrimitiveType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		, m_InputLayoutType(EInputLayoutType::StandardMesh)
		, m_CullMode(D3D12_CULL_MODE_BACK)
	{
		m_SourcePath = InSourcePath;
		Import();
	}
#endif
	
	Material::~Material()
	{
		if (m_RootSignature) m_RootSignature->Release();
		ReleasePSOs();

		if (m_ScalarCBV)
		{
			m_ScalarCBV->ReleaseBufferedResource();
			m_ScalarCBV = nullptr;
		}

		Renderer::Get()->TempSRVAllocator.Free(m_ScalarCpuHandle, m_ScalarGpuHandle);
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

			uint8 PrimitiveType;
			Ar >> PrimitiveType;
			m_PrimitiveType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(PrimitiveType);

			uint16 InputlayoutType;
			Ar >> InputlayoutType;
			m_InputLayoutType = static_cast<EInputLayoutType>(InputlayoutType);

			uint8 CullMode;
			Ar >> CullMode;
			m_CullMode = static_cast<D3D12_CULL_MODE>(CullMode);

			uint8 Texture2DCount;
			Ar >> Texture2DCount;
			m_Texture2DSlots.clear();
			m_Texture2DSlots.reserve(Texture2DCount);
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots.push_back(Texture2DProperty());
				m_Texture2DSlots[i].Serialize(Ar);
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

			InitalizeParameterMap();
		}

		else
		{
			Ar << m_SourcePath; 
			
			m_MainShaderBlob.Serialize(Ar);
			m_HitProxyShaderBlob.Serialize(Ar);

			Ar << static_cast<uint8>(m_PrimitiveType);
			Ar << static_cast<uint16>(m_InputLayoutType);
			Ar << static_cast<uint8>(m_CullMode);

			uint8 Texture2DCount = m_Texture2DSlots.size();
			Ar << Texture2DCount;
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots[i].Serialize(Ar);
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
		}
	}

	void Material::ReleaseShaderBlobs()
	{
		m_MainShaderBlob.ReleaseBlobs();
		m_HitProxyShaderBlob.ReleaseBlobs();
		m_EditorPrimitiveShaderBlob.ReleaseBlobs();
	}

	void Material::ReleasePSOs()
	{
		if (m_MainPassPSO)
		{
			m_MainPassPSO->ReleaseBufferedResource();
			m_MainPassPSO = nullptr;
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
		Renderer::Get()->Flush();
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
			SCOPE_STAT(UploadResourceMaterial);


			ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

			for (Texture2DProperty& Slot : m_Texture2DSlots)
			{
				if (!Slot.m_Texture2D.IsValid())
				{
					Slot.m_Texture2D.Load();
				}
			}

			if (m_RootSignature) m_RootSignature->Release();
			ReleasePSOs();
			
			if (m_ScalarCBV)
			{
				m_ScalarCBV->ReleaseBufferedResource();
				m_ScalarCBV = nullptr;
			}

			Renderer::Get()->TempSRVAllocator.Free(m_ScalarCpuHandle, m_ScalarGpuHandle);

			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			std::vector<D3D12_ROOT_PARAMETER> rootParameters;
			rootParameters.reserve(1 + 2 * m_Texture2DSlots.size());

			{
				D3D12_ROOT_PARAMETER Param = {};
				Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				Param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				Param.Constants.Num32BitValues = (sizeof( XMMATRIX ) / 4) + (sizeof( XMMATRIX ) / 4) + (sizeof(Guid) / 4);
				Param.Constants.ShaderRegister = 0;
				Param.Constants.RegisterSpace = 0;
				rootParameters.push_back(Param);
			}

			const int NumTexture2Ds = m_Texture2DSlots.size();
			const int ScalarCount = m_FloatSlots.size();
			const int Vector4Count = m_Vector4Slots.size();
			const bool NeedsConstantBuffer = ScalarCount > 0 || Vector4Count > 0;

			std::vector<D3D12_DESCRIPTOR_RANGE> Ranges;
			Ranges.reserve(NumTexture2Ds * 2 + NeedsConstantBuffer );

			for (int i = 0; i < NumTexture2Ds; i++)
			{
				{
					D3D12_DESCRIPTOR_RANGE Range = {};
					Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					Range.BaseShaderRegister = i;
					Range.RegisterSpace = 0;
					Range.NumDescriptors = 1;
					Range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					Ranges.push_back(Range);

					D3D12_ROOT_PARAMETER Param = {};
					Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					Param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					Param.DescriptorTable.NumDescriptorRanges = 1;
					Param.DescriptorTable.pDescriptorRanges = &Ranges[i * 2];

					rootParameters.push_back(Param);
				}

				{
					D3D12_DESCRIPTOR_RANGE Range = {};
					Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					Range.BaseShaderRegister = i;
					Range.RegisterSpace = 0;
					Range.NumDescriptors = 1;
					Range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
					Ranges.push_back(Range);

					D3D12_ROOT_PARAMETER Param = {};
					Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
					Param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
					Param.DescriptorTable.NumDescriptorRanges = 1;
					Param.DescriptorTable.pDescriptorRanges = &Ranges[i * 2 + 1];

					rootParameters.push_back(Param);
				}
			}

			if (NeedsConstantBuffer)
			{
				D3D12_DESCRIPTOR_RANGE Range = {};
				Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				Range.BaseShaderRegister = 1;
				Range.RegisterSpace = 0;
				Range.NumDescriptors = 1;
				Range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				Ranges.push_back(Range);

				D3D12_ROOT_PARAMETER Param = {};
				Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				Param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				Param.DescriptorTable.NumDescriptorRanges = 1;
				Param.DescriptorTable.pDescriptorRanges = &Ranges[NumTexture2Ds * 2];

				rootParameters.push_back(Param);

// ---------------------------------------------------------------------------------------------------------

				Renderer::Get()->TempSRVAllocator.Alloc(&m_ScalarCpuHandle, &m_ScalarGpuHandle);

				// TODO: maybe serialize this to asset
				std::vector<float> ConstantBuffer;
				ConstantBuffer.reserve(ScalarCount + Vector4Count * 4);
				for (int i = 0; i < ScalarCount; i++)
				{
					ConstantBuffer.push_back(m_FloatSlots[i].m_Value);
				}
				for (int i = 0; i < Vector4Count; i++)
				{
					ConstantBuffer.push_back(m_Vector4Slots[i].m_Value.GetX());
					ConstantBuffer.push_back(m_Vector4Slots[i].m_Value.GetY());
					ConstantBuffer.push_back(m_Vector4Slots[i].m_Value.GetZ());
					ConstantBuffer.push_back(m_Vector4Slots[i].m_Value.GetW());
				}

				const size_t ConstantBufferSize = ConstantBuffer.size() * sizeof( float );
				// 256 padding 
				const size_t ConstantBufferSizePadded = ( ConstantBufferSize + 255 ) & ~255;

				m_ScalarCBV = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( ConstantBufferSizePadded ), D3D12_RESOURCE_STATE_GENERIC_READ);

				UINT8* ConstantBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				m_ScalarCBV->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
				memcpy( ConstantBufferStart, ConstantBuffer.data(), ConstantBufferSizePadded );
				m_ScalarCBV->GetD3D12Resource()->Unmap(0, nullptr);

				D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
				ResourceViewDesc.BufferLocation = m_ScalarCBV->GetD3D12Resource()->GetGPUVirtualAddress();
				ResourceViewDesc.SizeInBytes = ConstantBufferSizePadded;
				Device->CreateConstantBufferView( &ResourceViewDesc , m_ScalarCpuHandle);
			}

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(rootParameters.size(), rootParameters.data(), 0, nullptr, rootSignatureFlags );

			ID3DBlob* pSerializedRootSig;
			ID3DBlob* pRootSigError;
			HRESULT Result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &pSerializedRootSig, &pRootSigError);
			if ( FAILED(Result) )
			{
				if ( pRootSigError )
				{
					LOG(LogMaterial, Error, "shader signature serialization failed. %s", (char*)pRootSigError->GetBufferPointer());
					pRootSigError->Release();
				}

				if (pSerializedRootSig)
				{
					pSerializedRootSig->Release();
					pSerializedRootSig = nullptr;
				}

				return;
			}

			std::string name = Path::ConvertShortPath(m_Path);
			name = Path::RemoveFileExtension(name);

			Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
				pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
#if D3D12_Debug_INFO
			m_RootSignature->SetName(StringHelper::s2ws(" RootSignature_" + name ).c_str());
#endif

			if (m_SupportMainPass)
			{
				m_MainPassPSO = PipelineStateObject::CreateMainPassPSO(m_RootSignature, m_CullMode, m_InputLayoutType,
					m_PrimitiveType, m_MainShaderBlob);

#if D3D12_Debug_INFO
				m_MainPassPSO->SetName( "MainPassPSO_" + name );
#endif
			}


#if WITH_EDITOR

			if (m_SupportEditorSelectionPass)
			{
				m_SelectionPassPSO = PipelineStateObject::CreateSelectionPassPSO(m_RootSignature, m_CullMode, m_InputLayoutType,
					m_PrimitiveType, m_MainShaderBlob);
#if D3D12_Debug_INFO
				m_SelectionPassPSO->SetName( "SelectionPassPSO_" + name );
#endif
			}

			if (IsSupportingHitProxyPass())
			{
				m_HitProxyPassPSO = PipelineStateObject::CreateHitProxyPassPSO(m_RootSignature, m_CullMode, m_InputLayoutType,
					m_PrimitiveType, m_HitProxyShaderBlob);

#if D3D12_Debug_INFO
				m_HitProxyPassPSO->SetName( "HitProxyPassPSO_" + name );
#endif
			}
#endif

			if (m_SupportEditorPrimitivePass)
			{
				m_EditorProxyPSO = PipelineStateObject::CreateEditorPrimitivePassPSO(m_RootSignature, m_CullMode, m_InputLayoutType,
					m_PrimitiveType, m_EditorPrimitiveShaderBlob);
#if D3D12_Debug_INFO
				m_EditorProxyPSO->SetName( "EditorPrimitivePSO_" + name );
#endif
			}

			ClearRenderStateDirty();
		}

		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			m_Texture2DSlots[i].m_Texture2D->UploadResources(CommandList);
		}
	}

	void Material::BindMainPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(m_RootSignature);
		CommandList->SetPipelineState(m_MainPassPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

	void Material::BindEditorPrimitivePass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(m_RootSignature);
		CommandList->SetPipelineState(m_EditorProxyPSO->GetD3D12PSO());

		BindResources(CommandList);
	}

#if WITH_EDITOR
	void Material::BindSelectionPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(m_RootSignature);
		CommandList->SetPipelineState(m_SelectionPassPSO->GetD3D12PSO());
		CommandList->OMSetStencilRef( 255 );

		BindResources(CommandList);
	}

	void Material::BindHitProxyPass( ID3D12GraphicsCommandList2* CommandList )
	{
		CommandList->SetGraphicsRootSignature(m_RootSignature);
		CommandList->SetPipelineState(m_HitProxyPassPSO->GetD3D12PSO());

		BindResources(CommandList);
	}
#endif

	void Material::BindResources( ID3D12GraphicsCommandList2* CommandList )
	{
		const int NumTexture2Ds = m_Texture2DSlots.size();
		for (int i = 0; i < m_Texture2DSlots.size(); i++)
		{
			CommandList->SetGraphicsRootDescriptorTable( 1 + i * 2, m_Texture2DSlots[i].m_Texture2D->SamplerGpuHandle );
			CommandList->SetGraphicsRootDescriptorTable( 2 + i * 2, m_Texture2DSlots[i].m_Texture2D->TextureGpuHandle );
		}

		const int NumScalars = m_FloatSlots.size();
		if (NumScalars > 0)
		{
			CommandList->SetGraphicsRootDescriptorTable( 1 + NumTexture2Ds * 2, m_ScalarGpuHandle);
		}
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


	void Material::SetNamedScalar( const std::string& Name, float Value )
	{
		auto it = m_ScalarMap.find(Name);

		if ( it != m_ScalarMap.end() )
		{
			it->second->m_Value = Value;

			if (m_ScalarCBV && m_ScalarCBV->GetD3D12Resource())
			{
				UINT8* ScalarBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				m_ScalarCBV->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ScalarBufferStart ) );
				memcpy( ScalarBufferStart + it->second->m_Index * sizeof(float), &Value, sizeof(float) );
				m_ScalarCBV->GetD3D12Resource()->Unmap(0, nullptr);
			}
		}
	}

	void Material::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		auto it = m_Vector4Map.find(Name);
		
		if ( it != m_Vector4Map.end() )
		{
			it->second->m_Value = Value;

			if (m_ScalarCBV && m_ScalarCBV->GetD3D12Resource())
			{
				UINT8* ScalarBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				m_ScalarCBV->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ScalarBufferStart ) );

				float Buffer[4] = { Value.GetX(), Value.GetY(), Value.GetZ(), Value.GetW() };
				memcpy( ScalarBufferStart + it->second->m_Index * sizeof(float), &Buffer[0], sizeof(float) * 4 );
				m_ScalarCBV->GetD3D12Resource()->Unmap(0, nullptr);
			}
		}
	}

}