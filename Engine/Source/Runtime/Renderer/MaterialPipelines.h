#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct MaterialPipelines : RefCountedObject
	{
		MaterialPipelines( D3D12CommandList* CommandList, class Material* InMaterial );

		TRefCountPtr<GraphicsPipelineState> m_MainPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_PrePassPSO;
		TRefCountPtr<GraphicsPipelineState> m_PointLightShadowDepthPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_SpotLightShadowDepthPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_DeferredDecalPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_StaticMeshDecalPassPSO;
		
#if WITH_EDITOR
		TRefCountPtr<GraphicsPipelineState> m_SelectionPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_HitProxyPassPSO;
		TRefCountPtr<GraphicsPipelineState> m_EditorProxyPSO;
#endif
	};
}