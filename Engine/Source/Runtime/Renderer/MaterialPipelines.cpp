#include "DrnPCH.h"
#include "MaterialPipelines.h"
#include "Runtime/Renderer/Material.h"
#include "Runtime/Renderer/RenderState.h"

namespace Drn
{
	static BoundShaderStateInput GetShaderStateInput(VertexDeclaration* VDeclaration, ShaderBlob& Blob)
	{
		VertexShader* VShader = nullptr;
		HullShader* HShader = nullptr;
		DomainShader* DShader = nullptr;
		PixelShader* PShader = nullptr;
		GeometryShader* GShader = nullptr;

		if (Blob.m_VS)
		{
			VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = Blob.m_VS->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = Blob.m_VS->GetBufferSize();
		}

		if (Blob.m_HS)
		{
			HShader = new HullShader();
			HShader->ByteCode.pShaderBytecode = Blob.m_HS->GetBufferPointer();
			HShader->ByteCode.BytecodeLength = Blob.m_HS->GetBufferSize();
		}

		if (Blob.m_DS)
		{
			DShader = new DomainShader();
			DShader->ByteCode.pShaderBytecode = Blob.m_DS->GetBufferPointer();
			DShader->ByteCode.BytecodeLength = Blob.m_DS->GetBufferSize();
		}

		if (Blob.m_PS)
		{
			PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = Blob.m_PS->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = Blob.m_PS->GetBufferSize();
		}

		if (Blob.m_GS)
		{
			GShader = new GeometryShader();
			GShader->ByteCode.pShaderBytecode = Blob.m_GS->GetBufferPointer();
			GShader->ByteCode.BytecodeLength = Blob.m_GS->GetBufferSize();
		}

		return BoundShaderStateInput(VDeclaration, VShader, HShader, DShader, PShader, GShader);
	}

	MaterialPipelines::MaterialPipelines( D3D12CommandList* CommandList, Material* InMaterial )
	{
		std::string name = Path::ConvertShortPath(InMaterial->m_Path);
		name = Path::RemoveFileExtension(name);

		//if (InMaterial->HasBasePass())
		//{
		//	BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_MainShaderBlob);
		//	TRefCountPtr<BlendState> BState = nullptr;
		//
		//	RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//	TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//	DepthStencilStateInitializer DInit(false, ECompareFunction::GreaterEqual);
		//	TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//	DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_COLOR_DEFERRED_FORMAT, GBUFFER_BASE_COLOR_FORMAT, GBUFFER_WORLD_NORMAL_FORMAT, GBUFFER_MASKS_FORMAT, GBUFFER_MASKS_FORMAT };
		//	ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//	GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//		_countof(TargetFormats), TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//	m_MainPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//	SetName(m_MainPassPSO->PipelineState, "PSO_MainPass_" + name);
		//}

		//if ( InMaterial->HasPrePass() && InMaterial->HasCustomPrePass() )
		//{
		//	BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_PrePassShaderBlob);
		//	TRefCountPtr<BlendState> BState = nullptr;
		//
		//	RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//	TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//	DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		//	TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//	DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
		//	ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//	GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//		0, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//	m_PrePassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//	SetName(m_PrePassPSO->PipelineState, "PSO_PrePass_" + name);
		//}

		if ( InMaterial->m_SupportDeferredDecalPass )
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_Pos, InMaterial->m_DeferredDecalShaderBlob);

			BlendStateInitializer BInit( {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::InverseSourceAlpha)} );
			BInit.bUseIndependentRenderTargetBlendStates = false;
			TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::Front);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DECAL_BASE_COLOR_FORMAT, DECAL_NORMAL_FORMAT, DECAL_MASKS_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				3, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_DeferredDecalPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_DeferredDecalPassPSO->PipelineState, "PSO_DecalPass_" + name);
		}

		if ( InMaterial->m_SupportStaticMeshDecalPass )
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_StaticMeshDecalShaderBlob);

			BlendStateInitializer BInit( {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::InverseSourceAlpha)} );
			BInit.bUseIndependentRenderTargetBlendStates = false;
			TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DECAL_BASE_COLOR_FORMAT, DECAL_NORMAL_FORMAT, DECAL_MASKS_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				3, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_StaticMeshDecalPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_StaticMeshDecalPassPSO->PipelineState, "PSO_StaticMeshDecalPass_" + name);
		}

		//if (InMaterial->HasShadowPass())
		//{
		//	{
		//		BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_PointlightShadowDepthShaderBlob);
		//		TRefCountPtr<BlendState> BState = nullptr;
		//
		//		RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//		TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//		DepthStencilStateInitializer DInit(true, ECompareFunction::LessEqual);
		//		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
		//		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//			0, TargetFormats, TargetFlags, DXGI_FORMAT_D16_UNORM, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//		m_PointLightShadowDepthPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//		SetName(m_PointLightShadowDepthPassPSO->PipelineState, "PSO_PointLightShadowDepthPass_" + name);
		//	}
		//
		//	{
		//		BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_SpotlightShadowDepthShaderBlob);
		//		TRefCountPtr<BlendState> BState = nullptr;
		//
		//		RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//		TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//		DepthStencilStateInitializer DInit(true, ECompareFunction::LessEqual);
		//		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
		//		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//			0, TargetFormats, TargetFlags, DXGI_FORMAT_D16_UNORM, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//		m_SpotLightShadowDepthPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//		SetName(m_SpotLightShadowDepthPassPSO->PipelineState, "PSO_SpotLightShadowDepthPass_" + name);
		//	}
		//}

#if WITH_EDITOR

		//if ( InMaterial->HasEditorSelectionPass() )
		//{
		//	BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_MainShaderBlob);
		//	BoundShaderState.m_PixelShader = nullptr;
		//		
		//	TRefCountPtr<BlendState> BState = nullptr;
		//
		//	RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//	TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//	DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual,
		//		true, ECompareFunction::Always, EStencilOp::Replace, EStencilOp::Replace, EStencilOp::Replace,
		//		true, ECompareFunction::Always, EStencilOp::Replace, EStencilOp::Replace, EStencilOp::Replace);
		//	TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//	DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
		//	ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//	GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//		0, TargetFormats, TargetFlags, DEPTH_STENCIL_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//	m_SelectionPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//	SetName(m_SelectionPassPSO->PipelineState, "PSO_SelectionPass_" + name);
		//}
		//
		//if ( InMaterial->HasHitProxyPass() )
		//{
		//	BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_HitProxyShaderBlob);
		//	TRefCountPtr<BlendState> BState = nullptr;
		//
		//	RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//	TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//	DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		//	TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//	DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_GUID_FORMAT };
		//	ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//	GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//		1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//	m_HitProxyPassPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//	SetName(m_HitProxyPassPSO->PipelineState, "PSO_HitProxyPass_" + name);
		//}
		//
		//if ( InMaterial->HasEditorPrimitivePass() )
		//{
		//	BoundShaderStateInput BoundShaderState = GetShaderStateInput(CommonResources::Get()->VertexDeclaration_StaticMesh, InMaterial->m_EditorPrimitiveShaderBlob);
		//	TRefCountPtr<BlendState> BState = nullptr;
		//
		//	RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->m_TwoSided ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
		//	TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);
		//
		//	DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		//	TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		//
		//	DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		//	ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };
		//
		//	GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
		//		1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);
		//
		//	m_EditorProxyPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		//	SetName(m_EditorProxyPSO->PipelineState, "PSO_EditorPrimitive_" + name);
		//}

#endif
	}
}