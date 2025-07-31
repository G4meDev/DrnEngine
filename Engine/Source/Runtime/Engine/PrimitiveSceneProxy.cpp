#include "DrnPCH.h"
#include "PrimitiveSceneProxy.h"

namespace Drn
{
	PrimitiveSceneProxy::PrimitiveSceneProxy( const PrimitiveComponent* InComponent )
		: m_EditorPrimitive( InComponent->IsEditorPrimitive() )
		, m_LocalToWorld( Matrix(InComponent->GetWorldTransform()) )
#if D3D12_Debug_INFO
		, m_Name( InComponent ? InComponent->GetComponentLabel() : "InvalidSceneProxy" )
#endif
	{
		
	}

	PrimitiveSceneProxy::~PrimitiveSceneProxy()
	{
		
	}

}