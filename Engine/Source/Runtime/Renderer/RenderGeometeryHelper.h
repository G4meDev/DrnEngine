#pragma once

namespace Drn
{
	class RenderGeometeryHelper
	{
	public:
		static void CreateSpotlightStencilGeometery(ID3D12GraphicsCommandList2* CommandList, VertexBuffer*& VB, IndexBuffer*& IB);
	};
}
