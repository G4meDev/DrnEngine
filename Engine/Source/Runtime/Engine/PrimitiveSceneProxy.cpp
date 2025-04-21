#include "DrnPCH.h"
#include "PrimitiveSceneProxy.h"

namespace Drn
{
	PrimitiveSceneProxy::PrimitiveSceneProxy( const PrimitiveComponent* InComponent )
	{
		
	}

	PrimitiveSceneProxy::~PrimitiveSceneProxy()
	{
		
	}

	void PrimitiveSceneProxy::Destroy()
	{
		delete this;
	}

}