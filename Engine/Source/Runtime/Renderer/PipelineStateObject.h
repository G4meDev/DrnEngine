#pragma once

#include "ForwardTypes.h"
#include "BufferedResource.h"

namespace Drn
{
	class PipelineStateObject : public BufferedResource
	{
	public:
		static PipelineStateObject* CreateMainPassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders);

		static PipelineStateObject* CreateDecalPassPSO( const ShaderBlob& Shaders);

		static PipelineStateObject* CreateMeshDecalPassPSO(D3D12_CULL_MODE CullMode, const ShaderBlob& Shaders);

		static PipelineStateObject* CreatePointLightShadowDepthPassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders);

		static PipelineStateObject* CreateSpotLightShadowDepthPassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders);

		static PipelineStateObject* CreateSelectionPassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders);

		static PipelineStateObject* CreateHitProxyPassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders);

		static PipelineStateObject* CreateEditorPrimitivePassPSO(D3D12_CULL_MODE CullMode, EInputLayoutType InputLayoutType,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveType, const ShaderBlob& Shaders);

		inline ID3D12PipelineState* GetD3D12PSO() { return m_PipelineState.Get(); }

#if D3D12_Debug_INFO
		void SetName(const std::string& Name);
#endif

	private:
		PipelineStateObject();
		virtual ~PipelineStateObject();
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	};
}