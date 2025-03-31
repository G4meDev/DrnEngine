#include "DrnPCH.h"
#include "CameraActor.h"

namespace Drn
{
	CameraActor::CameraActor()
		: Actor()
	{
		m_CameraComponenet = std::make_unique<CameraComponent>();
		GetRoot()->AttachSceneComponent(m_CameraComponenet.get());
	}

	CameraActor::~CameraActor()
	{
		
	}

	void CameraActor::Tick( float DeltaTime )
	{
		Actor::Tick(DeltaTime);


	}

}