#include "DrnPCH.h"
#include "Material.h"
#include "Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h"
#include "Editor/AssetImporter/AssetImporterMaterial.h"

namespace Drn
{
	Material::Material( const std::string& InPath )
		: Asset(InPath)
		, m_VS_Blob(nullptr)
		, m_PS_Blob(nullptr)
		, m_GS_Blob(nullptr)
		, m_HS_Blob(nullptr)
		, m_DS_Blob(nullptr)
		, m_CS_Blob(nullptr)
		, m_RootSignature(nullptr)
		, m_BasePassPSO(nullptr)
		, m_LoadedOnGPU(false)
		, m_PrimitiveType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		, m_InputLayoutType(EInputLayoutType::StandardMesh)
		, m_CullMode(D3D12_CULL_MODE_BACK)
	{
		Load();
	}

#if WITH_EDITOR
	Material::Material( const std::string& InPath, const std::string& InSourcePath )
		: Asset(InPath)
		, m_VS_Blob(nullptr)
		, m_PS_Blob(nullptr)
		, m_GS_Blob(nullptr)
		, m_HS_Blob(nullptr)
		, m_DS_Blob(nullptr)
		, m_CS_Blob(nullptr)
		, m_RootSignature(nullptr)
		, m_BasePassPSO(nullptr)
		, m_LoadedOnGPU(false)
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
		ReleaseShaderBlobs();
		if (m_RootSignature) m_RootSignature->Release();
		if (m_BasePassPSO) m_BasePassPSO->Release();
	}

	EAssetType Material::GetAssetType() { return EAssetType::Material; }

	void Material::Serialize( Archive& Ar )
	{
		Asset::Serialize(Ar);

		if (Ar.IsLoading())
		{
			ReleaseShaderBlobs();

			Ar >> m_SourcePath;
			Ar >> m_VS_Blob >> m_PS_Blob >> m_GS_Blob >> m_HS_Blob >> m_DS_Blob >> m_CS_Blob;

			uint8 PrimitiveType;
			Ar >> PrimitiveType;
			m_PrimitiveType = static_cast<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(PrimitiveType);

			uint16 InputlayoutType;
			Ar >> InputlayoutType;
			m_InputLayoutType = static_cast<EInputLayoutType>(InputlayoutType);

			uint8 CullMode;
			Ar >> CullMode;
			m_CullMode = static_cast<D3D12_CULL_MODE>(CullMode);
		}

		else
		{
			Ar << m_SourcePath; 
			Ar << m_VS_Blob << m_PS_Blob << m_GS_Blob << m_HS_Blob << m_DS_Blob << m_CS_Blob;
			Ar << static_cast<uint8>(m_PrimitiveType);
			Ar << static_cast<uint16>(m_InputLayoutType);
			Ar << static_cast<uint8>(m_CullMode);
		}
	}

	void Material::ReleaseShaderBlobs()
	{
		if (m_VS_Blob)
		{
			m_VS_Blob->Release();
			m_VS_Blob = nullptr;
		}

		if (m_PS_Blob)
		{
			m_PS_Blob->Release();
			m_PS_Blob = nullptr;
		}

		if (m_GS_Blob)
		{
			m_GS_Blob->Release();
			m_GS_Blob = nullptr;
		}

		if (m_HS_Blob)
		{
			m_HS_Blob->Release();
			m_HS_Blob = nullptr;
		}

		if (m_DS_Blob)
		{
			m_DS_Blob->Release();
			m_DS_Blob = nullptr;
		}

		if (m_CS_Blob)
		{
			m_CS_Blob->Release();
			m_CS_Blob = nullptr;
		}
	}

#if WITH_EDITOR
	void Material::Import()
	{
		AssetImporterMaterial::Import( this, m_SourcePath );
		Save();
		Load();

		m_LoadedOnGPU = false;
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
		SCOPE_STAT(UploadResourceMaterial);

		if (m_RootSignature) m_RootSignature->Release();
		if (m_BasePassPSO) m_BasePassPSO->Release();

		// TODO: figure something for deny flags
		//D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		//	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		//	D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		//rootParameters[0].InitAsConstants( sizeof( XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );
		rootParameters[0].InitAsConstants( sizeof( XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL );

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription(
			_countof( rootParameters ), rootParameters, 0, nullptr, rootSignatureFlags );

		ID3DBlob* pSerializedRootSig;
		ID3DBlob* pRootSigError;
		HRESULT Result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &pSerializedRootSig, &pRootSigError);
		if ( FAILED(Result) )
		{
			if ( pRootSigError )
			{
				//LOG(LogAssetImporterMaterial, Error, "Shader compile failed. %s", (char*)pRootSigError->GetBufferPointer());
				pRootSigError->Release();
			}

			if (pSerializedRootSig)
			{
				pSerializedRootSig->Release();
				pSerializedRootSig = nullptr;
			}

			return;
		}

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
			pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

		DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = m_CullMode;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= m_RootSignature;
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(m_InputLayoutType);
		PipelineDesc.PrimitiveTopologyType				= m_PrimitiveType;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_LESS;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		if (GetVS()) PipelineDesc.VS					= CD3DX12_SHADER_BYTECODE(GetVS());
		if (GetPS()) PipelineDesc.PS					= CD3DX12_SHADER_BYTECODE(GetPS());
		if (GetGS()) PipelineDesc.GS					= CD3DX12_SHADER_BYTECODE(GetGS());
		if (GetHS()) PipelineDesc.HS					= CD3DX12_SHADER_BYTECODE(GetHS());
		if (GetDS()) PipelineDesc.DS					= CD3DX12_SHADER_BYTECODE(GetDS());
		PipelineDesc.DSVFormat							= depthBufferFormat;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= backBufferFormat;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState(&PipelineDesc, IID_PPV_ARGS(&m_BasePassPSO));
		m_LoadedOnGPU = true;
	}

}