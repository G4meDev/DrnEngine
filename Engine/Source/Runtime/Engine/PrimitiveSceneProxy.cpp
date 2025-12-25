#include "DrnPCH.h"
#include "PrimitiveSceneProxy.h"

namespace Drn
{
	PrimitiveSceneProxy::PrimitiveSceneProxy( const PrimitiveComponent* InComponent )
		: m_LocalToWorld( Matrix(InComponent->GetWorldTransform()) )
#if WITH_EDITOR
		, m_EditorPrimitive( InComponent->IsEditorPrimitive() )
#endif
#if D3D12_Debug_INFO
		, m_Name( InComponent ? InComponent->GetComponentLabel() : "InvalidSceneProxy" )
#endif
	{
		
	}

	PrimitiveSceneProxy::~PrimitiveSceneProxy()
	{
		
	}

}