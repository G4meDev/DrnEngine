#include "DrnPCH.h"
#include "SphereReflectionCapture.h"
#include "Runtime/Engine/SphereReflectionCaptureComponent.h"

namespace Drn
{
	SphereReflectionCapture::SphereReflectionCapture()
		: ReflectionCapture()
	{
		m_SphereReflectionCaptureComponent = std::make_unique<SphereReflectionCaptureComponent>();
		m_SphereReflectionCaptureComponent->SetComponentLabel( "ReflectionCaptureComponent" );
		SetRootComponent(m_SphereReflectionCaptureComponent.get());
		m_CaptureComponent = m_SphereReflectionCaptureComponent.get();
	}

	SphereReflectionCapture::~SphereReflectionCapture()
	{
		
	}

	void SphereReflectionCapture::Serialize( Archive& Ar )
	{
		ReflectionCapture::Serialize(Ar);

		m_SphereReflectionCaptureComponent->Serialize(Ar);
	}

        }