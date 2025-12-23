#pragma once

#include "ForwardTypes.h"

namespace Drn
{
	struct VertexElement
	{
		uint8 StreamIndex;
		uint8 Offset;
		DXGI_FORMAT Format;
		uint8 AttributeIndex;
		uint16 Stride;
		const char* SemanticName;

		uint16 bUseInstanceIndex;

		VertexElement() {}
		VertexElement(uint8 InStreamIndex,uint8 InOffset,DXGI_FORMAT InForamt, const char* InSemanticName,uint8 InAttributeIndex,uint16 InStride,bool bInUseInstanceIndex = false):
			StreamIndex(InStreamIndex),
			Offset(InOffset),
			Format(InForamt),
			SemanticName(InSemanticName),
			AttributeIndex(InAttributeIndex),
			Stride(InStride),
			bUseInstanceIndex(bInUseInstanceIndex)
		{}
	};

	class VertexDeclaration : public SimpleRenderResource
	{
	public:

		static TRefCountPtr<VertexDeclaration> Create(const std::vector<VertexElement>& InElements);

		std::vector<D3D12_INPUT_ELEMENT_DESC> VertexElements;
		uint16 StreamStrides[MAX_VERTEX_ELEMENT_COUT];
	};
}