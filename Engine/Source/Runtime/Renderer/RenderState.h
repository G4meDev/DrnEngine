#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	class BlendStateInitializer
	{
	public:

		struct RenderTarget
		{
			EBlendOperation ColorBlendOp;
			EBlendFactor ColorSrcBlend;
			EBlendFactor ColorDestBlend;
			EBlendOperation AlphaBlendOp;
			EBlendFactor AlphaSrcBlend;
			EBlendFactor AlphaDestBlend;
			EColorWriteMask ColorWriteMask;
		
			RenderTarget(
				EBlendOperation InColorBlendOp = EBlendOperation::Add,
				EBlendFactor InColorSrcBlend = EBlendFactor::One,
				EBlendFactor InColorDestBlend = EBlendFactor::Zero,
				EBlendOperation InAlphaBlendOp = EBlendOperation::Add,
				EBlendFactor InAlphaSrcBlend = EBlendFactor::One,
				EBlendFactor InAlphaDestBlend = EBlendFactor::Zero,
				EColorWriteMask InColorWriteMask = EColorWriteMask::RGBA
				)
			: ColorBlendOp(InColorBlendOp)
			, ColorSrcBlend(InColorSrcBlend)
			, ColorDestBlend(InColorDestBlend)
			, AlphaBlendOp(InAlphaBlendOp)
			, AlphaSrcBlend(InAlphaSrcBlend)
			, AlphaDestBlend(InAlphaDestBlend)
			, ColorWriteMask(InColorWriteMask)
			{}

			//friend FArchive& operator<<(FArchive& Ar,FRenderTarget& RenderTarget)
			//{
			//	Ar << RenderTarget.ColorBlendOp;
			//	Ar << RenderTarget.ColorSrcBlend;
			//	Ar << RenderTarget.ColorDestBlend;
			//	Ar << RenderTarget.AlphaBlendOp;
			//	Ar << RenderTarget.AlphaSrcBlend;
			//	Ar << RenderTarget.AlphaDestBlend;
			//	Ar << RenderTarget.ColorWriteMask;
			//	return Ar;
			//}
		};

		BlendStateInitializer() {}

		BlendStateInitializer(const RenderTarget& InRenderTargetBlendState, bool bInUseAlphaToCoverage = false)
		:	bUseIndependentRenderTargetBlendStates(false)
		,	bUseAlphaToCoverage(bInUseAlphaToCoverage)
		{
			RenderTargets[0] = InRenderTargetBlendState;
		}

		template<uint32 NumRenderTargets>
		BlendStateInitializer(const RenderTarget (&InRenderTargetBlendStates)[NumRenderTargets], bool bInUseAlphaToCoverage = false)
			: bUseIndependentRenderTargetBlendStates(NumRenderTargets > 1)
			, bUseAlphaToCoverage(bInUseAlphaToCoverage)
		{
			static_assert(NumRenderTargets <= D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "Too many render target blend states.");

			for(uint32 RenderTargetIndex = 0;RenderTargetIndex < NumRenderTargets;++RenderTargetIndex)
			{
				RenderTargets[RenderTargetIndex] = InRenderTargetBlendStates[RenderTargetIndex];
			}
		}

		RenderTarget RenderTargets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		bool bUseIndependentRenderTargetBlendStates;
		bool bUseAlphaToCoverage;
	
		//friend FArchive& operator<<(FArchive& Ar,FBlendStateInitializerRHI& BlendStateInitializer)
		//{
		//	Ar << BlendStateInitializer.RenderTargets;
		//	Ar << BlendStateInitializer.bUseIndependentRenderTargetBlendStates;
		//	Ar << BlendStateInitializer.bUseAlphaToCoverage;
		//	return Ar;
		//}
	};

	class BlendState : public SimpleRenderResource
	{
	public:

		D3D12_BLEND_DESC Desc;

		static TRefCountPtr<BlendState> Create(const BlendStateInitializer& Init);
		//virtual bool GetInitializer(class FBlendStateInitializerRHI& Init) final override;
	};

// -------------------------------------------------------------------------------------------------------------------

	struct RasterizerStateInitializer
	{
		ERasterizerFillMode FillMode;
		ERasterizerCullMode CullMode;
		float DepthBias;
		float SlopeScaleDepthBias;
		bool bAllowMSAA;
		bool bEnableLineAA;

		//friend FArchive& operator<<(FArchive& Ar,FRasterizerStateInitializerRHI& RasterizerStateInitializer)
		//{
		//	Ar << RasterizerStateInitializer.FillMode;
		//	Ar << RasterizerStateInitializer.CullMode;
		//	Ar << RasterizerStateInitializer.DepthBias;
		//	Ar << RasterizerStateInitializer.SlopeScaleDepthBias;
		//	Ar << RasterizerStateInitializer.bAllowMSAA;
		//	Ar << RasterizerStateInitializer.bEnableLineAA;
		//	return Ar;
		//}

		//friend bool operator== (const FRasterizerStateInitializerRHI& A, const FRasterizerStateInitializerRHI& B);
	};

	class RasterizerState : public SimpleRenderResource
	{
	public:
		D3D12_RASTERIZER_DESC Desc;

		static TRefCountPtr<RasterizerState> Create(const RasterizerStateInitializer& Initializer);
		//virtual bool GetInitializer(struct FRasterizerStateInitializerRHI& Init) final override;
	};

// -------------------------------------------------------------------------------------------------------------------

	struct DepthStencilStateInitializer
	{
		bool bEnableDepthWrite;
		ECompareFunction DepthTest;

		bool bEnableFrontFaceStencil;
		ECompareFunction FrontFaceStencilTest;
		EStencilOp FrontFaceStencilFailStencilOp;
		EStencilOp FrontFaceDepthFailStencilOp;
		EStencilOp FrontFacePassStencilOp;
		bool bEnableBackFaceStencil;
		ECompareFunction BackFaceStencilTest;
		EStencilOp BackFaceStencilFailStencilOp;
		EStencilOp BackFaceDepthFailStencilOp;
		EStencilOp BackFacePassStencilOp;
		uint8 StencilReadMask;
		uint8 StencilWriteMask;

		DepthStencilStateInitializer(
			bool bInEnableDepthWrite = true,
			ECompareFunction InDepthTest = ECompareFunction::LessEqual,
			bool bInEnableFrontFaceStencil = false,
			ECompareFunction InFrontFaceStencilTest = ECompareFunction::Always,
			EStencilOp InFrontFaceStencilFailStencilOp = EStencilOp::Keep,
			EStencilOp InFrontFaceDepthFailStencilOp = EStencilOp::Keep,
			EStencilOp InFrontFacePassStencilOp = EStencilOp::Keep,
			bool bInEnableBackFaceStencil = false,
			ECompareFunction InBackFaceStencilTest = ECompareFunction::Always,
			EStencilOp InBackFaceStencilFailStencilOp = EStencilOp::Keep,
			EStencilOp InBackFaceDepthFailStencilOp = EStencilOp::Keep,
			EStencilOp InBackFacePassStencilOp = EStencilOp::Keep,
			uint8 InStencilReadMask = 0xFF,
			uint8 InStencilWriteMask = 0xFF
			)
		: bEnableDepthWrite(bInEnableDepthWrite)
		, DepthTest(InDepthTest)
		, bEnableFrontFaceStencil(bInEnableFrontFaceStencil)
		, FrontFaceStencilTest(InFrontFaceStencilTest)
		, FrontFaceStencilFailStencilOp(InFrontFaceStencilFailStencilOp)
		, FrontFaceDepthFailStencilOp(InFrontFaceDepthFailStencilOp)
		, FrontFacePassStencilOp(InFrontFacePassStencilOp)
		, bEnableBackFaceStencil(bInEnableBackFaceStencil)
		, BackFaceStencilTest(InBackFaceStencilTest)
		, BackFaceStencilFailStencilOp(InBackFaceStencilFailStencilOp)
		, BackFaceDepthFailStencilOp(InBackFaceDepthFailStencilOp)
		, BackFacePassStencilOp(InBackFacePassStencilOp)
		, StencilReadMask(InStencilReadMask)
		, StencilWriteMask(InStencilWriteMask)
		{}
	
		//friend FArchive& operator<<(FArchive& Ar,FDepthStencilStateInitializerRHI& DepthStencilStateInitializer)
		//{
		//	Ar << DepthStencilStateInitializer.bEnableDepthWrite;
		//	Ar << DepthStencilStateInitializer.DepthTest;
		//	Ar << DepthStencilStateInitializer.bEnableFrontFaceStencil;
		//	Ar << DepthStencilStateInitializer.FrontFaceStencilTest;
		//	Ar << DepthStencilStateInitializer.FrontFaceStencilFailStencilOp;
		//	Ar << DepthStencilStateInitializer.FrontFaceDepthFailStencilOp;
		//	Ar << DepthStencilStateInitializer.FrontFacePassStencilOp;
		//	Ar << DepthStencilStateInitializer.bEnableBackFaceStencil;
		//	Ar << DepthStencilStateInitializer.BackFaceStencilTest;
		//	Ar << DepthStencilStateInitializer.BackFaceStencilFailStencilOp;
		//	Ar << DepthStencilStateInitializer.BackFaceDepthFailStencilOp;
		//	Ar << DepthStencilStateInitializer.BackFacePassStencilOp;
		//	Ar << DepthStencilStateInitializer.StencilReadMask;
		//	Ar << DepthStencilStateInitializer.StencilWriteMask;
		//	return Ar;
		//}
		//
		//RHI_API friend uint32 GetTypeHash(const FDepthStencilStateInitializerRHI& Initializer);
		//RHI_API friend bool operator== (const FDepthStencilStateInitializerRHI& A, const FDepthStencilStateInitializerRHI& B);
		//
		//RHI_API FString ToString() const;
		//RHI_API void FromString(const FString& Src);
		//RHI_API void FromString(const FStringView& Src);
	};

	class DepthStencilState : public SimpleRenderResource
	{
	public:

		//D3D12_DEPTH_STENCIL_DESC1 Desc;
		D3D12_DEPTH_STENCIL_DESC Desc;
		EDepthStencilViewType AccessType;

		static TRefCountPtr<DepthStencilState> Create(const DepthStencilStateInitializer& Initializer);
		//virtual bool GetInitializer(struct FDepthStencilStateInitializerRHI& Init) final override;
	};

}