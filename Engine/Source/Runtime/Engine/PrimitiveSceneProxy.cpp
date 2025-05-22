#include "DrnPCH.h"
#include "PrimitiveSceneProxy.h"

namespace Drn
{
	PrimitiveSceneProxy::PrimitiveSceneProxy( const PrimitiveComponent* InComponent )
		: m_EditorPrimitive( InComponent->IsEditorPrimitive() )
	{
		
	}

	PrimitiveSceneProxy::~PrimitiveSceneProxy()
	{
		
	}

}