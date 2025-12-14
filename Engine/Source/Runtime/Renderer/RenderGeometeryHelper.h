#pragma once

namespace Drn
{
	class RenderGeometeryHelper
	{
	public:
		static void CreateSpotlightStencilGeometery(class D3D12CommandList* CommandList, class RenderVertexBuffer*& VB, class RenderIndexBuffer*& IB, uint32& VertexCount, uint32& PrimitiveCount);
	};
}
