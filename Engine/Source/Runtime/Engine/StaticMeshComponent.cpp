#include "DrnPCH.h"
#include "StaticMeshComponent.h"

namespace Drn
{
	void StaticMeshComponent::UploadResources( dx12lib::CommandList* CommandList )
	{
		m_VertexBuffer = CommandList->CopyVertexBuffer( VertexData.size(), sizeof(VertexPosColor), VertexData.data() );
		m_IndexBuffer = CommandList->CopyIndexBuffer(IndicesData.size(), DXGI_FORMAT_R16_UINT, IndicesData.data());
	}


}