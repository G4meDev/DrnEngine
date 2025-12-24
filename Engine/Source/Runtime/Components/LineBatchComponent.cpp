#include "DrnPCH.h"
#include "LineBatchComponent.h"

#include "Runtime/Core/Application.h"
#include "Runtime/Renderer/RenderBuffer.h"
#include <algorithm/for_each.hpp>

using namespace Microsoft::WRL;

namespace Drn
{
	void LineBatchComponent::TickComponent( float DeltaTime )
	{
		SCOPE_STAT();

		// @BUG: tags doesn't work in worker threads
		OPTICK_TAG("Name", GetComponentLabel().c_str());

		const size_t OldSize = m_Lines.size();

		std::_Erase_remove_if( m_Lines, [&](BatchLine& Line)
			{
				if (Line.RemainingLifetime < 0) { return true; }
				else { Line.RemainingLifetime -= DeltaTime; return false; }
			});

		if (m_Lines.size() != OldSize)
		{
			MarkRenderStateDirty();
		}
	}

	void LineBatchComponent::RegisterComponent( World* InOwningWorld )
	{
		PrimitiveComponent::RegisterComponent(InOwningWorld);

		m_SceneProxy = new LineBatchSceneProxy(this);
		InOwningWorld->GetScene()->RegisterPrimitiveProxy(m_SceneProxy);
	}

	void LineBatchComponent::UnRegisterComponent()
	{
		//m_SceneProxy->
		if (GetWorld()->GetScene())
		{
			GetWorld()->GetScene()->UnRegisterPrimitiveProxy(m_SceneProxy);
		}

		//delete m_SceneProxy;

		PrimitiveComponent::UnRegisterComponent();
	}

	void LineBatchComponent::DrawLine( const Vector& Start, const Vector& End, const Color& Color, float Thickness, float Lifetime )
	{
		m_Lines.push_back(BatchLine(Start, End, Color, Thickness, Lifetime));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawLines( const std::vector<BatchLine>& InLines )
	{
		m_Lines.insert(std::end(m_Lines), std::begin(InLines), std::end(InLines));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawCircle( const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime)
	{
		const float	AngleDelta = 2.0f * Math::PI / NumSides;
		Vector	LastVertex = Base + X * Radius;

		for(int32 SideIndex = 0;SideIndex < NumSides;SideIndex++)
		{
			const Vector Vertex = Base + (X * Math::Cos(AngleDelta * (SideIndex + 1)) + Z * Math::Sin(AngleDelta * (SideIndex + 1))) * Radius;
			m_Lines.push_back(BatchLine(LastVertex, Vertex, Color, Thickness, Lifetime));

			LastVertex = Vertex;
		}

		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawHalfCircle( const Vector& Base, const Vector& X, const Vector& Z, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		const float	AngleDelta = 2.0f * Math::PI / NumSides;
		Vector	LastVertex = Base + X * Radius;

		for(int32 SideIndex = 0;SideIndex < (NumSides/2);SideIndex++)
		{
			const Vector Vertex = Base + (X * Math::Cos(AngleDelta * (SideIndex + 1)) + Z * Math::Sin(AngleDelta * (SideIndex + 1))) * Radius;
			m_Lines.push_back(BatchLine(LastVertex, Vertex, Color, Thickness, Lifetime));

			LastVertex = Vertex;
		}

		MarkRenderStateDirty();

	}

	void LineBatchComponent::DrawSphere( const Vector& Center, const Quat& Rotation, const Color& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		Vector ForwardVector = Rotation.RotateVector(Vector::ForwardVector);
		Vector UpVector = Rotation.RotateVector(Vector::UpVector);
		Vector RightVector = Rotation.RotateVector(Vector::RightVector);

		DrawCircle(Center, ForwardVector, RightVector, Color, Radius, NumSides, Thickness, Lifetime);
		DrawCircle(Center, RightVector, UpVector, Color, Radius, NumSides, Thickness, Lifetime);
		DrawCircle(Center, UpVector, ForwardVector, Color, Radius, NumSides, Thickness, Lifetime);
	}

	void LineBatchComponent::DrawBox( const Box& InBox, const Transform& T, const Color& Color, float Thickness, float Lifetime )
	{
		Vector	B[2];
		int32 ai, aj;
		B[0] = InBox.Min;
		B[1] = InBox.Max;

		float P_X, P_Y, P_Z, Q_X, Q_Y, Q_Z;

		for( ai=0; ai<2; ai++ ) for( aj=0; aj<2; aj++ )
		{
			P_X=B[ai].GetX(); Q_X=B[ai].GetX();
			P_Y=B[aj].GetY(); Q_Y=B[aj].GetY();
			P_Z=B[0].GetZ()	; Q_Z=B[1].GetZ();
			DrawLine(T.TransformPosition(Vector(P_X, P_Y, P_Z)), T.TransformPosition(Vector(Q_X, Q_Y, Q_Z)), Color, Thickness, Lifetime);

			P_Y=B[ai].GetY(); Q_Y=B[ai].GetY();
			P_Z=B[aj].GetZ(); Q_Z=B[aj].GetZ();
			P_X=B[0].GetX()	; Q_X=B[1].GetX();
			DrawLine(T.TransformPosition(Vector(P_X, P_Y, P_Z)), T.TransformPosition(Vector(Q_X, Q_Y, Q_Z)), Color, Thickness, Lifetime);

			P_Z=B[ai].GetZ(); Q_Z=B[ai].GetZ();
			P_X=B[aj].GetX(); Q_X=B[aj].GetX();
			P_Y=B[0].GetY()	; Q_Y=B[1].GetY();
			DrawLine(T.TransformPosition(Vector(P_X, P_Y, P_Z)), T.TransformPosition(Vector(Q_X, Q_Y, Q_Z)), Color, Thickness, Lifetime);
		}
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawCapsule( const Vector& Center, float HalfHeight, float Radius, const Quat& Rotation, const Color& Color, float Thickness, float Lifetime )
	{
		const int32 NumSides = 16;

		Matrix Axes = Matrix(Transform(Vector::ZeroVector, Rotation));
		Vector AxisX = Axes.Rotation().GetAxisX();
		Vector AxisY = Axes.Rotation().GetAxisY();
		Vector AxisZ = Axes.Rotation().GetAxisZ();

		float HalfAxis = std::max(HalfHeight - Radius, 0.01f);
		Vector TopEnd = Center + AxisY * HalfAxis;
		Vector ButtomEnd = Center - AxisY * HalfAxis;

		DrawCircle(TopEnd, AxisX, AxisZ, Color, Radius, NumSides, Thickness, Lifetime);
		DrawCircle(ButtomEnd, AxisX, AxisZ, Color, Radius, NumSides, Thickness, Lifetime);

		DrawHalfCircle(TopEnd, AxisZ, AxisY, Color, Radius, NumSides, Thickness, Lifetime);
		DrawHalfCircle(TopEnd, AxisX, AxisY, Color, Radius, NumSides, Thickness, Lifetime);

		Vector NegAxisY = AxisY * -1;

		DrawHalfCircle(ButtomEnd, AxisZ, NegAxisY, Color, Radius, NumSides, Thickness, Lifetime);
		DrawHalfCircle(ButtomEnd, AxisX, NegAxisY, Color, Radius, NumSides, Thickness, Lifetime);

		DrawLine(TopEnd + AxisX * Radius, ButtomEnd + AxisX * Radius, Color, Thickness, Lifetime);
		DrawLine(TopEnd - AxisX * Radius, ButtomEnd - AxisX * Radius, Color, Thickness, Lifetime);
		DrawLine(TopEnd + AxisZ * Radius, ButtomEnd + AxisZ * Radius, Color, Thickness, Lifetime);
		DrawLine(TopEnd - AxisZ * Radius, ButtomEnd - AxisZ * Radius, Color, Thickness, Lifetime);
	}

	void LineBatchComponent::Flush()
	{
		m_Lines.clear();
		MarkRenderStateDirty();
	}

	void LineBatchComponent::SetThickness( bool InThickness )
	{
		m_Thickness = InThickness;
		if (m_SceneProxy)
		{
			m_SceneProxy->m_Thickness = m_Thickness;
		}
	}

// --------------------------------------------------------------------------------------------------------------------------------

	LineBatchSceneProxy::LineBatchSceneProxy( LineBatchComponent* InLineBatchComponent )
		: PrimitiveSceneProxy(InLineBatchComponent)
		, m_LineComponent(InLineBatchComponent)
		, m_VertexBuffer(nullptr)
		, m_Thickness(InLineBatchComponent->m_Thickness)
		, m_VertexCount(0)
	{
		m_VertexData.resize(MaxNumVertex);
	}

	LineBatchSceneProxy::~LineBatchSceneProxy()
	{}

	void LineBatchSceneProxy::RenderMainPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

	}

	void LineBatchSceneProxy::RenderPrePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();

	}

	void LineBatchSceneProxy::RenderShadowPass( D3D12CommandList* CommandList, SceneRenderer* Renderer, LightSceneProxy* LightProxy )
	{
		
	}

	void LineBatchSceneProxy::RenderDecalPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

#if WITH_EDITOR
	void LineBatchSceneProxy::RenderHitProxyPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void LineBatchSceneProxy::RenderSelectionPass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void LineBatchSceneProxy::RenderEditorPrimitivePass( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT();
		
		if (!m_HasValidData)
		{
			return;
		}
		
		CommandList->GetD3D12CommandList()->SetGraphicsRootSignature( Renderer::Get()->m_BindlessRootSinature.Get() );
		if (m_Thickness)
		{
			CommandList->SetGraphicPipelineState( CommonResources::Get()->m_DebugLineThicknessPSO->m_PSO);
		}
		else
		{
			CommandList->SetGraphicPipelineState( CommonResources::Get()->m_DebugLinePSO->m_PSO);
		}
		
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant( 0, Renderer->ViewBuffer->GetViewIndex(), 0 );

		uint16 const Strides[] = { sizeof(InputLayout_LineColorThickness) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawPrimitive(0, m_VertexCount / 2, 1);
	}

#endif

	void LineBatchSceneProxy::InitResources( D3D12CommandList* CommandList )
	{}

	void LineBatchSceneProxy::UpdateResources( D3D12CommandList* CommandList )
	{
		SCOPE_STAT();

		if (GetPrimitive()->IsRenderStateDirty())
		{
			RecalculateVertexData();
			UploadVertexBuffer(CommandList);

			GetPrimitive()->ClearRenderStateDirty();
		}
	}

	void LineBatchSceneProxy::RecalculateVertexData()
	{
		SCOPE_STAT();

		//const uint32 LineCount = m_LineComponent->m_Lines.size();
		//m_VertexCount = LineCount * 2;
		//
		//static tf::Taskflow w;
		//w.clear();
		//
		//tf::IndexRange<uint32> R(0, LineCount, 10000);
		//auto t = w.for_each_by_index(R, [&](tf::IndexRange<uint32> Subrange)
		//{
		//	SCOPE_STAT( RecaulculateDataGrain );
		//	for (uint32 i = Subrange.begin(); i != std::min( LineCount, Subrange.end()); i++)
		//	{
		//		const BatchLine& Line = m_LineComponent->m_Lines[i];
		//		
		//		const uint32 StartIndex = i * 2;
		//		const uint32 EndIndex = i * 2 + 1;
		//		
		//		m_VertexData[StartIndex].Set( Line.Start, Line.Color, Line.Thickness);
		//		m_VertexData[EndIndex].Set(Line.End, Line.Color, Line.Thickness);
		//	}
		//});

		//Application::executor.run(w).wait();
		//Application::executor.corun(w);

		const uint32 LineCount = m_LineComponent->m_Lines.size();
		m_VertexCount = LineCount * 2;

		for (uint32 i = 0; i < LineCount; i++)
		{
			const BatchLine& Line = m_LineComponent->m_Lines[i];
		
			const uint32 StartIndex = i * 2;
			const uint32 EndIndex = i * 2 + 1;

			m_VertexData[StartIndex].Set( Line.Start, Line.Color, Line.Thickness);
			m_VertexData[EndIndex].Set(Line.End, Line.Color, Line.Thickness);
		};
	}

	void LineBatchSceneProxy::UploadVertexBuffer(D3D12CommandList* CommandList)
	{
		if ( m_VertexCount > 0 )
		{
			SCOPE_STAT( "CopyBuffer" );

			{
				uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Dynamic;
				RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, m_VertexData.data(), ClearValueBinding::Black, "VB_LineBatchVertexBuffer");
				m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(InputLayout_LineColorThickness) * m_VertexCount, VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
			}

			m_HasValidData = true;
		}

		else
		{
			m_HasValidData = false;
		}
	}

}