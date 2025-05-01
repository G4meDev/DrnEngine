#include "DrnPCH.h"
#include "LineBatchComponent.h"

using namespace Microsoft::WRL;

#define NUM_MAX_LINES 10000

namespace Drn
{
	void LineBatchComponent::TickComponent( float DeltaTime )
	{
		bool Dirty = false;

		for ( auto it = m_Lines.begin(); it != m_Lines.end(); )
		{
			BatchLine& Line = *it;

			// at least draw for single frame
			if ( Line.RemainingLifetime < 0 )
			{
				it = m_Lines.erase(it);
				Dirty = true;
			}
			
			else
			{
				Line.RemainingLifetime -= DeltaTime;
				it++;
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

		delete m_SceneProxy;

		PrimitiveComponent::UnRegisterComponent();
	}

	void LineBatchComponent::DrawLine( const Vector& Start, const Vector& End, const Vector& Color, float Thickness, float Lifetime )
	{
		m_Lines.push_back(BatchLine(Start, End, Color, Thickness, Lifetime));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawLines( const std::vector<BatchLine>& InLines )
	{
		m_Lines.insert(std::end(m_Lines), std::begin(InLines), std::end(InLines));
		MarkRenderStateDirty();
	}

	void LineBatchComponent::DrawCircle( const Vector& Base, const Vector& X, const Vector& Z, const Vector& Color, float Radius, int32 NumSides, float Thickness, float Lifetime)
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

	void LineBatchComponent::DrawSphere( const Vector& Center, const Quat& Rotation, const Vector& Color, float Radius, int32 NumSides, float Thickness, float Lifetime )
	{
		Vector ForwardVector = Rotation.RotateVector(Vector::ForwardVector);
		Vector UpVector = Rotation.RotateVector(Vector::UpVector);
		Vector RightVector = Rotation.RotateVector(Vector::RightVector);

		DrawCircle(Center, ForwardVector, RightVector, Color, Radius, NumSides, Thickness, Lifetime);
		DrawCircle(Center, RightVector, UpVector, Color, Radius, NumSides, Thickness, Lifetime);
		DrawCircle(Center, UpVector, ForwardVector, Color, Radius, NumSides, Thickness, Lifetime);
	}

	void LineBatchComponent::DrawBox( const Box& InBox, const Transform& T, const Vector& Color, float Thickness, float Lifetime )
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
		m_LineBatchMaterial = AssetHandle<Material>( "Engine\\Content\\Materials\\M_LineBatch.drn" );
		m_LineBatchMaterial.Load();
	}

	LineBatchSceneProxy::~LineBatchSceneProxy()
	{
		//m_VertexBuffer.Reset();
		//m_IndexBuffer.Reset();
	}

	void LineBatchSceneProxy::RenderMainPass( dx12lib::CommandList* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT(LineBatchSceneProxyRenderMainPass);

		if (!m_HasValidData)
		{
			return;
		}

		CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(m_LineBatchMaterial->GetRootSignature());
		CommandList->GetD3D12CommandList()->SetPipelineState(m_LineBatchMaterial->GetBasePassPSO());
		CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );

		XMMATRIX modelMatrix = Matrix().Get();

		auto viewport = Renderer->m_RenderTarget.GetViewport();
		float    aspectRatio = viewport.Width / viewport.Height;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

		CommandList->GetD3D12CommandList()->IASetVertexBuffers( 0, 1, &m_VertexBufferView );
		CommandList->GetD3D12CommandList()->IASetIndexBuffer( &m_IndexBufferView );
		uint32 VertexCount = m_IndexBufferView.SizeInBytes / sizeof(uint32);
		CommandList->DrawIndexed( VertexCount );
	}

	void LineBatchSceneProxy::InitResources( dx12lib::CommandList* CommandList )
	{
		CommandList->GetDevice().GetD3D12Device()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer( sizeof(VertexData_LineColorThickness) * NUM_MAX_LINES * 2),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS( &m_VertexBuffer ) );

		m_VertexBuffer->SetName(L"LineBatchVertexBuffer");

		CommandList->GetDevice().GetD3D12Device()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer( sizeof(uint32) * NUM_MAX_LINES * 2),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS( &m_IndexBuffer ) );

#if D3D12_Debug_INFO
		m_IndexBuffer->SetName(L"LineBatchIndexBuffer");
#endif
	}

	void LineBatchSceneProxy::UpdateResources( dx12lib::CommandList* CommandList )
	{
		SCOPE_STAT(UpdateResourcesLineBatchComponent);

		if (GetPrimitive()->IsRenderStateDirty())
		{
			RecalculateVertexData();
			UploadVertexBuffer();

			GetPrimitive()->ClearRenderStateDirty();
		}

		if (!m_LineBatchMaterial->IsLoadedOnGpu())
		{
			m_LineBatchMaterial->UploadResources(CommandList);
		}
	}

	void LineBatchSceneProxy::RecalculateVertexData()
	{
		SCOPE_STAT( RecaulculateData );

		m_VertexData.clear();
		m_VertexData.reserve( m_LineComponent->m_Lines.size() * 2 );

		m_IndexData.clear();
		m_IndexData.reserve( m_LineComponent->m_Lines.size() * 2 );

		for ( uint32 i = 0; i < m_LineComponent->m_Lines.size(); i++ )
		{
			const BatchLine& Line = m_LineComponent->m_Lines[i];

			m_VertexData.push_back( VertexData_LineColorThickness( Line.Start, Line.Color, Line.Thickness) );
			m_VertexData.push_back( VertexData_LineColorThickness( Line.End, Line.Color, Line.Thickness) );

			m_IndexData.push_back( i * 2 );
			m_IndexData.push_back( i * 2 + 1 );
		}
	}

	void LineBatchSceneProxy::UploadVertexBuffer()
	{
		if ( m_VertexData.size() > 0 && m_IndexData.size() > 0 )
		{
			SCOPE_STAT( CopyBuffer );

			{
				UINT8*        pVertexDataBegin;
				CD3DX12_RANGE readRange( 0, 0 );
				m_VertexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) );
				uint32 ByteSize = m_VertexData.size() * sizeof( VertexData_LineColorThickness );
				memcpy( pVertexDataBegin, &m_VertexData[0], ByteSize );
				m_VertexBuffer->Unmap( 0, nullptr );

				m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
				m_VertexBufferView.StrideInBytes  = sizeof( VertexData_LineColorThickness );
				m_VertexBufferView.SizeInBytes    = ByteSize;
			}

			{
				UINT8*        pIndexDataBegin;
				CD3DX12_RANGE readRange( 0, 0 );
				m_IndexBuffer->Map( 0, &readRange, reinterpret_cast<void**>( &pIndexDataBegin ) );
				uint32 ByteSize = m_IndexData.size() * sizeof( uint32 );
				memcpy( pIndexDataBegin, &m_IndexData[0], ByteSize );
				m_IndexBuffer->Unmap( 0, nullptr );

				m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
				m_IndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
				m_IndexBufferView.SizeInBytes    = ByteSize;
			}

			m_HasValidData = true;
		}

		else
		{
			m_HasValidData = false;
		}
	}

}