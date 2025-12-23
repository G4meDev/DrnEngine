#include "DrnPCH.h"
#include "VertexDeclaration.h"

namespace Drn
{
	TRefCountPtr<VertexDeclaration> VertexDeclaration::Create( const std::vector<VertexElement>& InElements )
	{
		VertexDeclaration* OutVertexDeclaration = new VertexDeclaration;

		uint16 UsedStreamsMask = 0;
		memset(OutVertexDeclaration->StreamStrides, 0, sizeof(StreamStrides));

		for (int32 ElementIndex = 0; ElementIndex < InElements.size(); ElementIndex++)
		{
			const VertexElement& Element = InElements[ElementIndex];
			D3D12_INPUT_ELEMENT_DESC D3DElement = { 0 };
			D3DElement.InputSlot = Element.StreamIndex;
			D3DElement.AlignedByteOffset = Element.Offset;
			D3DElement.Format = Element.Format;
			D3DElement.SemanticName = Element.SemanticName;

			D3DElement.SemanticIndex = Element.AttributeIndex;
			D3DElement.InputSlotClass = Element.bUseInstanceIndex ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

			D3DElement.InstanceDataStepRate = Element.bUseInstanceIndex ? 1 : 0;

			if ((UsedStreamsMask & 1 << Element.StreamIndex) != 0)
			{
				drn_check(OutVertexDeclaration->StreamStrides[Element.StreamIndex] == Element.Stride);
			}
			else
			{
				UsedStreamsMask = UsedStreamsMask | (1 << Element.StreamIndex);
				OutVertexDeclaration->StreamStrides[Element.StreamIndex] = Element.Stride;
			}

			OutVertexDeclaration->VertexElements.push_back(D3DElement);
		}

		std::sort( OutVertexDeclaration->VertexElements.begin(), OutVertexDeclaration->VertexElements.end(),
			[](const D3D12_INPUT_ELEMENT_DESC& A, const D3D12_INPUT_ELEMENT_DESC &B)
			{
				return ((int32)A.AlignedByteOffset + A.InputSlot * UINT16_MAX) < ((int32)B.AlignedByteOffset + B.InputSlot * UINT16_MAX);
			});

		return OutVertexDeclaration;
	}


}