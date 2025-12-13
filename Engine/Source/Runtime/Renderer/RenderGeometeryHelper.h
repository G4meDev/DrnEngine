#pragma once

namespace Drn
{
	class RenderGeometeryHelper
	{
	public:
		static void CreateSpotlightStencilGeometery(class D3D12CommandList* CommandList, VertexBuffer*& VB, class RenderIndexBuffer*& IB);
	};
}
