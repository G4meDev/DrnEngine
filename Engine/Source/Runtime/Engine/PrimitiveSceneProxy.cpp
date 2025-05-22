#include "DrnPCH.h"
#include "PrimitiveSceneProxy.h"

namespace Drn
{
	PrimitiveSceneProxy::PrimitiveSceneProxy( const PrimitiveComponent* InComponent )
		: m_EditorPrimitive( InComponent->IsEditorPrimitive() )
		, m_LocalToWorld( Matrix(InComponent->GetWorldTransform()) )
	{
		
	}

	PrimitiveSceneProxy::~PrimitiveSceneProxy()
	{
		
	}

}