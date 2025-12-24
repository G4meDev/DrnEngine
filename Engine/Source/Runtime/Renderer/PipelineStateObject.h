#pragma once

#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderResource.h"
#include "Runtime/Renderer/Shader.h"
#include "Runtime/Renderer/RenderState.h"

namespace Drn
{
	class PipelineStateObject : public SimpleRenderResource
	{
	public:
		virtual uint32 AddRef() const { return SimpleRenderResource::AddRef(); }
		virtual uint32 Release() const { return SimpleRenderResource::Release(); }
		virtual uint32 GetRefCount() const { return SimpleRenderResource::GetRefCount(); }

		inline ID3D12PipelineState* GetD3D12PSO() { return m_PipelineState.Get(); }

#if D3D12_Debug_INFO
		void SetName(const std::string& Name);
#endif

	private:
		PipelineStateObject();
		virtual ~PipelineStateObject();
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	};

// ---------------------------------------------------------------------------------------------------------------------



	class GraphicsPipelineStateInitializer
	{
	public:

		GraphicsPipelineStateInitializer()
			: BlendState(nullptr)
			, RasterizerState(nullptr)
			, DepthStencilState(nullptr)
			, RenderTargetsEnabled(0)
			, RenderTargetFormats()
			, RenderTargetFlags()
			, DepthStencilTargetFormat(DXGI_FORMAT_UNKNOWN)
			, DepthStencilTargetFlag()
			, NumSamples(0)
			//, bDepthBounds(false)
			//, MultiViewCount(0)
			//, bHasFragmentDensityAttachment(false)
			//, ShadingRate(EVRSShadingRate::VRSSR_1x1)
			//, Flags(0)
		{}

		GraphicsPipelineStateInitializer(
			BoundShaderStateInput		InBoundShaderState,
			BlendState*					InBlendState,
			RasterizerState*			InRasterizerState,
			DepthStencilState*			InDepthStencilState,
			EPrimitiveType				InPrimitiveType,
			uint32						InRenderTargetsEnabled,
			const DXGI_FORMAT			(&InRenderTargetFormats)[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT],
			const ETextureCreateFlags	(&InRenderTargetFlags)[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT],
			DXGI_FORMAT					InDepthStencilTargetFormat,
			ETextureCreateFlags			InDepthStencilTargetFlag,
			EDepthStencilViewType		InDepthStencilAccess,
			uint32						InNumSamples)
			//uint16						InFlags,
			//bool						bInDepthBounds,
			//uint8						InMultiViewCount,
			//bool						bInHasFragmentDensityAttachment,
			//EVRSShadingRate				InShadingRate)
			: BoundShaderState(InBoundShaderState)
			, BlendState(InBlendState)
			, RasterizerState(InRasterizerState)
			, DepthStencilState(InDepthStencilState)
			, PrimitiveType(InPrimitiveType)
			, RenderTargetsEnabled(InRenderTargetsEnabled)
			//, RenderTargetFormats(InRenderTargetFormats)
			//, RenderTargetFlags(InRenderTargetFlags)
			, DepthStencilTargetFormat(InDepthStencilTargetFormat)
			, DepthStencilTargetFlag(InDepthStencilTargetFlag)
			, DepthStencilAccess(InDepthStencilAccess)
			, NumSamples(InNumSamples)
			//, bDepthBounds(bInDepthBounds)
			//, MultiViewCount(InMultiViewCount)
			//, bHasFragmentDensityAttachment(bInHasFragmentDensityAttachment)
			//, ShadingRate(InShadingRate)
			//, Flags(InFlags)
		{
			memcpy(RenderTargetFormats, InRenderTargetFormats, sizeof(RenderTargetFormats));
			memcpy(RenderTargetFlags, InRenderTargetFlags, sizeof(RenderTargetFlags));
		}

		BoundShaderStateInput			BoundShaderState;
		BlendState*						BlendState;
		RasterizerState*				RasterizerState;
		DepthStencilState*				DepthStencilState;
	
		EPrimitiveType					PrimitiveType;
		uint32							RenderTargetsEnabled;
		DXGI_FORMAT						RenderTargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		ETextureCreateFlags				RenderTargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		DXGI_FORMAT						DepthStencilTargetFormat;
		ETextureCreateFlags				DepthStencilTargetFlag;
		EDepthStencilViewType			DepthStencilAccess;
		uint16							NumSamples;
		//bool							bDepthBounds;
		//uint8							MultiViewCount;
		//bool							bHasFragmentDensityAttachment;
		//EVRSShadingRate				ShadingRate;
	};
	
	class GraphicsPipelineState : public SimpleRenderResource
	{
	private:
		GraphicsPipelineState(const GraphicsPipelineStateInitializer& Initializer, ID3D12RootSignature* InRootSignature);
		virtual ~GraphicsPipelineState();

	public:	
		static TRefCountPtr <GraphicsPipelineState> Create(class Device* InDevice, const GraphicsPipelineStateInitializer& Initializer, ID3D12RootSignature* InRootSignature);

		GraphicsPipelineStateInitializer PipelineStateInitializer;
		ID3D12RootSignature* RootSignature;
		uint16 StreamStrides[MAX_VERTEX_ELEMENT_COUT];

		TRefCountPtr<ID3D12PipelineState> PipelineState;

		inline class VertexShader*   GetVertexShader() const { return PipelineStateInitializer.BoundShaderState.m_VertexShader; }
		inline class PixelShader*    GetPixelShader() const { return PipelineStateInitializer.BoundShaderState.m_PixelShader; }
		inline class HullShader*     GetHullShader() const { return PipelineStateInitializer.BoundShaderState.m_HullShader; }
		inline class DomainShader*   GetDomainShader() const { return PipelineStateInitializer.BoundShaderState.m_DomainShader; }
		inline class GeometryShader* GetGeometryShader() const { return PipelineStateInitializer.BoundShaderState.m_GeometryShader; }
	};
}