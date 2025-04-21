#include "DrnPCH.h"
#include "LineBatchComponent.h"

namespace Drn
{
	void LineBatchComponent::TickComponent( float DeltaTime )
	{
		bool Dirty = false;

		for ( auto it = m_Lines.begin(); it != m_Lines.end(); )
		{
			BatchLine& Line = *it;

			if ( Line.RemainingLifetime >0.0f )
			{
				Line.RemainingLifetime -= DeltaTime;

				if (Line.RemainingLifetime <= 0.0f)
				{
					it = m_Lines.erase(it);
					Dirty = true;
				}
			}
		}

		if (Dirty)
		{
			MarkRenderStateDirty();
		}
	}

	void LineBatchComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		m_SceneProxy = new LineBatchSceneProxy(this);
		InOwningWorld->GetScene()->AddPrimitiveProxy(m_SceneProxy);
	}

	void LineBatchComponent::UnRegisterComponent()
	{
		if (GetWorld()->GetScene())
		{
			GetWorld()->GetScene()->RemovePrimitiveProxy(m_SceneProxy);
		}

		PrimitiveComponent::UnRegisterComponent();
	}

	void LineBatchComponent::DrawLine( const Vector& Start, const Vector& End, const Vector& Color, float Lifetime )
	{
		m_Lines.push_back(BatchLine(Start, End, Color, Lifetime));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawLines( const std::vector<BatchLine>& InLines )
	{
		m_Lines.insert(std::end(m_Lines), std::begin(InLines), std::end(InLines));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::Flush()
	{
		m_Lines.clear();
		MarkRenderStateDirty();
	}

// --------------------------------------------------------------------------------------------------------------------------------

	LineBatchSceneProxy::LineBatchSceneProxy( LineBatchComponent* InLineBatchComponent )
		: PrimitiveSceneProxy(InLineBatchComponent)
		, m_LineComponent(InLineBatchComponent)
	{
		
	}

	LineBatchSceneProxy::~LineBatchSceneProxy()
	{
		
	}

	void LineBatchSceneProxy::RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer ) const
	{
		
	}

	void LineBatchSceneProxy::UpdateResources(dx12lib::CommandList* CommandList)
	{
		 

	}

}