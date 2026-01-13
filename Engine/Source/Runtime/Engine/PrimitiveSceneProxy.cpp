#include "DrnPCH.h"
#include "PrimitiveSceneProxy.h"

namespace Drn
{
	PrimitiveSceneProxy::PrimitiveSceneProxy( const PrimitiveComponent* InComponent )
		: m_LocalToWorld( Matrix(InComponent->GetWorldTransform()) )
		, bStatic(InComponent->IsStatic())
		, MinDrawDistance(0)
		, MaxDrawDistance(0)
		, bPendingDestory(false)
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