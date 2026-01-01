#include "DrnPCH.h"
#include "RenderState.h"

namespace Drn
{
	static D3D12_TEXTURE_ADDRESS_MODE TranslateAddressMode(ESamplerAddressMode AddressMode)
	{
		switch (AddressMode)
		{
		case ESamplerAddressMode::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case ESamplerAddressMode::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case ESamplerAddressMode::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		};
	}

	static D3D12_COMPARISON_FUNC TranslateSamplerCompareFunction(ESamplerCompareFunction SamplerComparisonFunction)
	{
		switch (SamplerComparisonFunction)
		{
		case ESamplerCompareFunction::Less: return D3D12_COMPARISON_FUNC_LESS;
		case ESamplerCompareFunction::Never:
		default: return D3D12_COMPARISON_FUNC_NEVER;
		};
	}

	static D3D12_BLEND_OP TranslateBlendOp(EBlendOperation BlendOp)
	{
		switch (BlendOp)
		{
		case EBlendOperation::Subtract: return D3D12_BLEND_OP_SUBTRACT;
		case EBlendOperation::Min: return D3D12_BLEND_OP_MIN;
		case EBlendOperation::Max: return D3D12_BLEND_OP_MAX;
		case EBlendOperation::ReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
		default: return D3D12_BLEND_OP_ADD;
		};
	}

	static EBlendOperation ReverseTranslateBlendOp(D3D12_BLEND_OP BlendOp)
	{
		switch (BlendOp)
		{
		case D3D12_BLEND_OP_SUBTRACT: return EBlendOperation::Subtract;
		case D3D12_BLEND_OP_MIN: return EBlendOperation::Min;
		case D3D12_BLEND_OP_MAX: return EBlendOperation::Max;
		case D3D12_BLEND_OP_REV_SUBTRACT: return EBlendOperation::ReverseSubtract;
		default: return EBlendOperation::Add;
		};
	}

	static D3D12_BLEND TranslateBlendFactor(EBlendFactor BlendFactor)
	{
		switch (BlendFactor)
		{
		case EBlendFactor::One: return D3D12_BLEND_ONE;
		case EBlendFactor::SourceColor: return D3D12_BLEND_SRC_COLOR;
		case EBlendFactor::InverseSourceColor: return D3D12_BLEND_INV_SRC_COLOR;
		case EBlendFactor::SourceAlpha: return D3D12_BLEND_SRC_ALPHA;
		case EBlendFactor::InverseSourceAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
		case EBlendFactor::DestAlpha: return D3D12_BLEND_DEST_ALPHA;
		case EBlendFactor::InverseDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
		case EBlendFactor::DestColor: return D3D12_BLEND_DEST_COLOR;
		case EBlendFactor::InverseDestColor: return D3D12_BLEND_INV_DEST_COLOR;
		case EBlendFactor::ConstantBlendFactor: return D3D12_BLEND_BLEND_FACTOR;
		case EBlendFactor::InverseConstantBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
		case EBlendFactor::Source1Color: return D3D12_BLEND_SRC1_COLOR;
		case EBlendFactor::InverseSource1Color: return D3D12_BLEND_INV_SRC1_COLOR;
		case EBlendFactor::Source1Alpha: return D3D12_BLEND_SRC1_ALPHA;
		case EBlendFactor::InverseSource1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
		default: return D3D12_BLEND_ZERO;
		};
	}

	static EBlendFactor ReverseTranslateBlendFactor(D3D12_BLEND BlendFactor)
	{
		switch (BlendFactor)
		{
		case D3D12_BLEND_ONE: return EBlendFactor::One;
		case D3D12_BLEND_SRC_COLOR: return EBlendFactor::SourceColor;
		case D3D12_BLEND_INV_SRC_COLOR: return EBlendFactor::InverseSourceColor;
		case D3D12_BLEND_SRC_ALPHA: return EBlendFactor::SourceAlpha;
		case D3D12_BLEND_INV_SRC_ALPHA: return EBlendFactor::InverseSourceAlpha;
		case D3D12_BLEND_DEST_ALPHA: return EBlendFactor::DestAlpha;
		case D3D12_BLEND_INV_DEST_ALPHA: return EBlendFactor::InverseDestAlpha;
		case D3D12_BLEND_DEST_COLOR: return EBlendFactor::DestColor;
		case D3D12_BLEND_INV_DEST_COLOR: return EBlendFactor::InverseDestColor;
		case D3D12_BLEND_BLEND_FACTOR: return EBlendFactor::ConstantBlendFactor;
		case D3D12_BLEND_INV_BLEND_FACTOR: return EBlendFactor::InverseConstantBlendFactor;
		case D3D12_BLEND_SRC1_COLOR: return EBlendFactor::Source1Color;
		case D3D12_BLEND_INV_SRC1_COLOR: return EBlendFactor::InverseSource1Color;
		case D3D12_BLEND_SRC1_ALPHA: return EBlendFactor::Source1Alpha;
		case D3D12_BLEND_INV_SRC1_ALPHA: return EBlendFactor::InverseSource1Alpha;
		default: return EBlendFactor::Zero;
		};
	}


	static D3D12_CULL_MODE TranslateCullMode(ERasterizerCullMode CullMode)
	{
		switch (CullMode)
		{
		case ERasterizerCullMode::Back : return D3D12_CULL_MODE_BACK;
		case ERasterizerCullMode::Front : return D3D12_CULL_MODE_FRONT;
		default: return D3D12_CULL_MODE_NONE;
		};
	}

	static ERasterizerCullMode ReverseTranslateCullMode(D3D12_CULL_MODE CullMode)
	{
		switch (CullMode)
		{
		case D3D12_CULL_MODE_BACK: return ERasterizerCullMode::Back;
		case D3D12_CULL_MODE_FRONT: return ERasterizerCullMode::Front;
		default: return ERasterizerCullMode::None;
		}
	}

	static D3D12_FILL_MODE TranslateFillMode(ERasterizerFillMode FillMode)
	{
		switch (FillMode)
		{
		case ERasterizerFillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
		default: return D3D12_FILL_MODE_SOLID;
		};
	}

	static ERasterizerFillMode ReverseTranslateFillMode(D3D12_FILL_MODE FillMode)
	{
		switch (FillMode)
		{
		case D3D12_FILL_MODE_WIREFRAME: return ERasterizerFillMode::Wireframe;
		default: return ERasterizerFillMode::Solid;
		}
	}


	static D3D12_COMPARISON_FUNC TranslateCompareFunction(ECompareFunction CompareFunction)
	{
		switch (CompareFunction)
		{
		case ECompareFunction::Less: return D3D12_COMPARISON_FUNC_LESS;
		case ECompareFunction::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case ECompareFunction::Greater: return D3D12_COMPARISON_FUNC_GREATER;
		case ECompareFunction::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case ECompareFunction::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
		case ECompareFunction::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case ECompareFunction::Never: return D3D12_COMPARISON_FUNC_NEVER;
		default: return D3D12_COMPARISON_FUNC_ALWAYS;
		};
	}

	static ECompareFunction ReverseTranslateCompareFunction(D3D12_COMPARISON_FUNC CompareFunction)
	{
		switch (CompareFunction)
		{
		case D3D12_COMPARISON_FUNC_LESS: return ECompareFunction::Less;
		case D3D12_COMPARISON_FUNC_LESS_EQUAL: return ECompareFunction::LessEqual;
		case D3D12_COMPARISON_FUNC_GREATER: return ECompareFunction::Greater;
		case D3D12_COMPARISON_FUNC_GREATER_EQUAL: return ECompareFunction::GreaterEqual;
		case D3D12_COMPARISON_FUNC_EQUAL: return ECompareFunction::Equal;
		case D3D12_COMPARISON_FUNC_NOT_EQUAL: return ECompareFunction::NotEqual;
		case D3D12_COMPARISON_FUNC_NEVER: return ECompareFunction::Never;
		default: return ECompareFunction::Always;
		}
	}


	static D3D12_STENCIL_OP TranslateStencilOp(EStencilOp StencilOp)
	{
		switch (StencilOp)
		{
		case EStencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
		case EStencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
		case EStencilOp::SaturatedIncrement: return D3D12_STENCIL_OP_INCR_SAT;
		case EStencilOp::SaturatedDecrement: return D3D12_STENCIL_OP_DECR_SAT;
		case EStencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
		case EStencilOp::Increment: return D3D12_STENCIL_OP_INCR;
		case EStencilOp::Decrement: return D3D12_STENCIL_OP_DECR;
		default: return D3D12_STENCIL_OP_KEEP;
		};
	}

	static EStencilOp ReverseTranslateStencilOp(D3D12_STENCIL_OP StencilOp)
	{
		switch (StencilOp)
		{
		case D3D12_STENCIL_OP_ZERO: return EStencilOp::Zero;
		case D3D12_STENCIL_OP_REPLACE: return EStencilOp::Replace;
		case D3D12_STENCIL_OP_INCR_SAT: return EStencilOp::SaturatedIncrement;
		case D3D12_STENCIL_OP_DECR_SAT: return EStencilOp::SaturatedDecrement;
		case D3D12_STENCIL_OP_INVERT: return EStencilOp::Invert;
		case D3D12_STENCIL_OP_INCR: return EStencilOp::Increment;
		case D3D12_STENCIL_OP_DECR: return EStencilOp::Decrement;
		default: return EStencilOp::Keep;
		};
	}


	TRefCountPtr<BlendState> BlendState::Create( const BlendStateInitializer& Init )
	{
		BlendState* OutBlendState = new BlendState;
		D3D12_BLEND_DESC &BlendDesc = OutBlendState->Desc;
		memset(&BlendDesc, 0, sizeof(D3D12_BLEND_DESC));

		BlendDesc.AlphaToCoverageEnable = Init.bUseAlphaToCoverage;
		BlendDesc.IndependentBlendEnable = Init.bUseIndependentRenderTargetBlendStates;

		for (uint32 RenderTargetIndex = 0; RenderTargetIndex < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++RenderTargetIndex)
		{
			const BlendStateInitializer::RenderTarget& RenderTargetInitializer = Init.RenderTargets[RenderTargetIndex];
			D3D12_RENDER_TARGET_BLEND_DESC& RenderTarget = BlendDesc.RenderTarget[RenderTargetIndex];
			RenderTarget.BlendEnable =
				RenderTargetInitializer.ColorBlendOp != EBlendOperation::Add || RenderTargetInitializer.ColorDestBlend != EBlendFactor::Zero || RenderTargetInitializer.ColorSrcBlend != EBlendFactor::One ||
				RenderTargetInitializer.AlphaBlendOp != EBlendOperation::Add || RenderTargetInitializer.AlphaDestBlend != EBlendFactor::Zero || RenderTargetInitializer.AlphaSrcBlend != EBlendFactor::One;

			RenderTarget.BlendOp = TranslateBlendOp(RenderTargetInitializer.ColorBlendOp);
			RenderTarget.SrcBlend = TranslateBlendFactor(RenderTargetInitializer.ColorSrcBlend);
			RenderTarget.DestBlend = TranslateBlendFactor(RenderTargetInitializer.ColorDestBlend);
			RenderTarget.BlendOpAlpha = TranslateBlendOp(RenderTargetInitializer.AlphaBlendOp);
			RenderTarget.SrcBlendAlpha = TranslateBlendFactor(RenderTargetInitializer.AlphaSrcBlend);
			RenderTarget.DestBlendAlpha = TranslateBlendFactor(RenderTargetInitializer.AlphaDestBlend);

			RenderTarget.RenderTargetWriteMask =
				  (EnumHasAnyFlags(RenderTargetInitializer.ColorWriteMask, EColorWriteMask::RED) ? D3D12_COLOR_WRITE_ENABLE_RED : 0)
				| (EnumHasAnyFlags(RenderTargetInitializer.ColorWriteMask, EColorWriteMask::GREEN) ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0)
				| (EnumHasAnyFlags(RenderTargetInitializer.ColorWriteMask, EColorWriteMask::BLUE) ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0)
				| (EnumHasAnyFlags(RenderTargetInitializer.ColorWriteMask, EColorWriteMask::ALPHA) ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
		}

		return OutBlendState;
	}

	TRefCountPtr<RasterizerState> RasterizerState::Create( const RasterizerStateInitializer& Initializer )
	{
		RasterizerState* OutRasterizerState = new RasterizerState;

		D3D12_RASTERIZER_DESC& RasterizerDesc = OutRasterizerState->Desc;
		memset(&RasterizerDesc, 0, sizeof(D3D12_RASTERIZER_DESC));

		RasterizerDesc.CullMode = TranslateCullMode(Initializer.CullMode);
		RasterizerDesc.FillMode = TranslateFillMode(Initializer.FillMode);
		RasterizerDesc.SlopeScaledDepthBias = Initializer.SlopeScaleDepthBias;
		RasterizerDesc.FrontCounterClockwise = false;
		RasterizerDesc.DepthBias = std::floor(Initializer.DepthBias * (float)(1 << 24));
		RasterizerDesc.DepthClipEnable = true;
		RasterizerDesc.MultisampleEnable = Initializer.bAllowMSAA;

		return OutRasterizerState;
	}

	TRefCountPtr<DepthStencilState> DepthStencilState::Create( const DepthStencilStateInitializer& Initializer )
	{
		DepthStencilState* OutDepthStencilState = new DepthStencilState;

		//D3D12_DEPTH_STENCIL_DESC1 &DepthStencilDesc = OutDepthStencilState->Desc;
		D3D12_DEPTH_STENCIL_DESC &DepthStencilDesc = OutDepthStencilState->Desc;
		memset(&DepthStencilDesc, 0, sizeof(D3D12_DEPTH_STENCIL_DESC1));

		DepthStencilDesc.DepthEnable = (Initializer.DepthTest != ECompareFunction::Always) || Initializer.bEnableDepthWrite;
		DepthStencilDesc.DepthWriteMask = Initializer.bEnableDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		DepthStencilDesc.DepthFunc = TranslateCompareFunction(Initializer.DepthTest);

		DepthStencilDesc.StencilEnable = Initializer.bEnableFrontFaceStencil || Initializer.bEnableBackFaceStencil;
		DepthStencilDesc.StencilReadMask = Initializer.StencilReadMask;
		DepthStencilDesc.StencilWriteMask = Initializer.StencilWriteMask;
		DepthStencilDesc.FrontFace.StencilFunc = TranslateCompareFunction(Initializer.FrontFaceStencilTest);
		DepthStencilDesc.FrontFace.StencilFailOp = TranslateStencilOp(Initializer.FrontFaceStencilFailStencilOp);
		DepthStencilDesc.FrontFace.StencilDepthFailOp = TranslateStencilOp(Initializer.FrontFaceDepthFailStencilOp);
		DepthStencilDesc.FrontFace.StencilPassOp = TranslateStencilOp(Initializer.FrontFacePassStencilOp);
		if (Initializer.bEnableBackFaceStencil)
		{
			DepthStencilDesc.BackFace.StencilFunc = TranslateCompareFunction(Initializer.BackFaceStencilTest);
			DepthStencilDesc.BackFace.StencilFailOp = TranslateStencilOp(Initializer.BackFaceStencilFailStencilOp);
			DepthStencilDesc.BackFace.StencilDepthFailOp = TranslateStencilOp(Initializer.BackFaceDepthFailStencilOp);
			DepthStencilDesc.BackFace.StencilPassOp = TranslateStencilOp(Initializer.BackFacePassStencilOp);
		}
		else
		{
			DepthStencilDesc.BackFace = DepthStencilDesc.FrontFace;
		}

		//DepthStencilDesc.DepthBoundsTestEnable = false;

		const bool bStencilOpIsKeep =
			Initializer.FrontFaceStencilFailStencilOp == EStencilOp::Keep
			&& Initializer.FrontFaceDepthFailStencilOp == EStencilOp::Keep
			&& Initializer.FrontFacePassStencilOp == EStencilOp::Keep
			&& Initializer.BackFaceStencilFailStencilOp == EStencilOp::Keep
			&& Initializer.BackFaceDepthFailStencilOp == EStencilOp::Keep
			&& Initializer.BackFacePassStencilOp == EStencilOp::Keep;

		const bool bMayWriteStencil = Initializer.StencilWriteMask != 0 && !bStencilOpIsKeep;

		EDepthStencilViewType AccessType;
		if (Initializer.bEnableDepthWrite && bMayWriteStencil)
		{
			AccessType = EDepthStencilViewType::DepthStencilWrite;
		}

		else if (Initializer.bEnableDepthWrite && !bMayWriteStencil)
		{
			AccessType = EDepthStencilViewType::DepthWrite;
		}

		else if (!Initializer.bEnableDepthWrite && bMayWriteStencil)
		{
			AccessType = EDepthStencilViewType::StencilWrite;
		}

		else
		{
			AccessType = EDepthStencilViewType::DepthStencilRead;
		}

		OutDepthStencilState->AccessType = AccessType;

		return OutDepthStencilState;
	}

	SamplerState::SamplerState( Device* InParent, const D3D12_SAMPLER_DESC& Desc )
		: DeviceChild(InParent)
	{
		CpuHandle.ptr = 0;
		CpuHandle = GetParentDevice()->GetSamplerDescriptorAllocator().AllocateHeapSlot(DescriptorHeapIndex, GpuHandle.ptr, Index);
		GetParentDevice()->GetD3D12Device()->CreateSampler(&Desc, CpuHandle);
	}
	
	SamplerState::~SamplerState()
	{
		if (CpuHandle.ptr)
		{
			GetParentDevice()->GetSamplerDescriptorAllocator().FreeHeapSlot(CpuHandle, DescriptorHeapIndex);
		}
	}
	
	TRefCountPtr<SamplerState> SamplerState::Create( Device* InParent, const SamplerStateInitializer& Initializer )
	{
		D3D12_SAMPLER_DESC SamplerDesc;
		memset(&SamplerDesc, 0, sizeof(D3D12_SAMPLER_DESC));

		SamplerDesc.AddressU = TranslateAddressMode(Initializer.AddressU);
		SamplerDesc.AddressV = TranslateAddressMode(Initializer.AddressV);
		SamplerDesc.AddressW = TranslateAddressMode(Initializer.AddressW);
		SamplerDesc.MipLODBias = Initializer.MipBias;
		SamplerDesc.MaxAnisotropy = Renderer::Get()->ComputeAnisotropy(Initializer.MaxAnisotropy);
		SamplerDesc.MinLOD = Initializer.MinMipLevel;
		SamplerDesc.MaxLOD = Initializer.MaxMipLevel;

		const bool bComparisonEnabled = Initializer.SamplerComparisonFunction != ESamplerCompareFunction::Never;
		switch (Initializer.Filter)
		{
		case ESamplerFilter::AnisotropicLinear:
		case ESamplerFilter::AnisotropicPoint:
			if (SamplerDesc.MaxAnisotropy == 1)
			{
				SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
			else
			{
				SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
			}

			break;
		case ESamplerFilter::Trilinear:
			SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case ESamplerFilter::Bilinear:
			SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case ESamplerFilter::Point:
			SamplerDesc.Filter = bComparisonEnabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
			break;
		}
		const Vector4 LinearBorderColor = Initializer.BorderColor.AsLinearVector();
		SamplerDesc.BorderColor[0] = LinearBorderColor.GetX();
		SamplerDesc.BorderColor[1] = LinearBorderColor.GetY();
		SamplerDesc.BorderColor[2] = LinearBorderColor.GetZ();
		SamplerDesc.BorderColor[3] = LinearBorderColor.GetW();
		SamplerDesc.ComparisonFunc = TranslateSamplerCompareFunction(Initializer.SamplerComparisonFunction);

		return new SamplerState(InParent, SamplerDesc);

		// TODO: add sampler cache
		//FScopeLock Lock(&GD3D12SamplerStateCacheLock);
		//
		//// Check to see if the sampler has already been created
		//// This is done to reduce cache misses accessing sampler objects
		//TRefCountPtr<FD3D12SamplerState>* PreviouslyCreated = SamplerMap.Find(SamplerDesc);
		//if (PreviouslyCreated)
		//{
		//	return PreviouslyCreated->GetReference();
		//}
		//else
		//{
		//	// 16-bit IDs are used for faster hashing
		//	check(SamplerID < 0xffff);
		//
		//	FD3D12SamplerState* NewSampler = new FD3D12SamplerState(this, SamplerDesc, static_cast<uint16>(SamplerID));
		//
		//	SamplerMap.Add(SamplerDesc, NewSampler);
		//
		//	SamplerID++;
		//
		//	INC_DWORD_STAT(STAT_UniqueSamplers);
		//
		//	return NewSampler;
		//}
	}

 }  // namespace Drn