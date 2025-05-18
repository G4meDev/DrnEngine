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
		, m_VertexBufferResource(nullptr)
		, m_IndexBufferResource(nullptr)
	{
		m_LineBatchMaterial = AssetHandle<Material>( "Engine\\Content\\Materials\\M_LineBatch.drn" );
		m_LineBatchMaterial.Load();
	}

	LineBatchSceneProxy::~LineBatchSceneProxy()
	{
		if (m_VertexBufferResource)
		{
			m_VertexBufferResource->ReleaseBufferedResource();
		}

		if (m_IndexBufferResource)
		{
			m_IndexBufferResource->ReleaseBufferedResource();
		}
	}

	void LineBatchSceneProxy::RenderMainPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		SCOPE_STAT(LineBatchSceneProxyRenderMainPass);

		if (!m_HasValidData)
		{
			return;
		}

		m_LineBatchMaterial->BindMainPass(CommandList);
		CommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );

		XMMATRIX modelMatrix = Matrix().Get();

		float aspectRatio = (float)( Renderer->GetViewportSize().X ) / Renderer->GetViewportSize().Y;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		Renderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphicsRoot32BitConstants( 0, 16, &mvpMatrix, 0 );

		CommandList->IASetVertexBuffers( 0, 1, &m_VertexBufferView );
		CommandList->IASetIndexBuffer( &m_IndexBufferView );
		uint32 VertexCount = m_IndexBufferView.SizeInBytes / sizeof(uint32);
		CommandList->DrawIndexedInstanced( VertexCount, 1, 0, 0, 0);
	}

	void LineBatchSceneProxy::RenderSelectionPass( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		
	}

	void LineBatchSceneProxy::InitResources( ID3D12GraphicsCommandList2* CommandList )
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		m_VertexBufferResource = Resource::Create( D3D12_HEAP_TYPE_UPLOAD,
			CD3DX12_RESOURCE_DESC::Buffer( sizeof(InputLayout_LineColorThickness) * NUM_MAX_LINES * 2),
			D3D12_RESOURCE_STATE_GENERIC_READ );

#if D3D12_Debug_INFO
		m_VertexBufferResource->SetName("LineBatchVertexBuffer");
#endif

		m_IndexBufferResource = Resource::Create( D3D12_HEAP_TYPE_UPLOAD,
			CD3DX12_RESOURCE_DESC::Buffer( sizeof( uint32 ) * NUM_MAX_LINES * 2 ),
			D3D12_RESOURCE_STATE_GENERIC_READ );

#if D3D12_Debug_INFO
		m_IndexBufferResource->SetName("LineBatchIndexBuffer");
#endif
	}

	void LineBatchSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
	{
		SCOPE_STAT(UpdateResourcesLineBatchComponent);

		if (GetPrimitive()->IsRenderStateDirty())
		{
			RecalculateVertexData();
			UploadVertexBuffer();

			GetPrimitive()->ClearRenderStateDirty();
		}

		m_LineBatchMaterial->UploadResources(CommandList);
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

			m_VertexData.push_back( InputLayout_LineColorThickness( Line.Start, Line.Color, Line.Thickness) );
			m_VertexData.push_back( InputLayout_LineColorThickness( Line.End, Line.Color, Line.Thickness) );

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
				m_VertexBufferResource->GetD3D12Resource()->Map( 0, &readRange, reinterpret_cast<void**>( &pVertexDataBegin ) );
				uint32 ByteSize = m_VertexData.size() * sizeof( InputLayout_LineColorThickness );
				memcpy( pVertexDataBegin, &m_VertexData[0], ByteSize );
				m_VertexBufferResource->GetD3D12Resource()->Unmap( 0, nullptr );

				m_VertexBufferView.BufferLocation = m_VertexBufferResource->GetD3D12Resource()->GetGPUVirtualAddress();
				m_VertexBufferView.StrideInBytes  = sizeof( InputLayout_LineColorThickness );
				m_VertexBufferView.SizeInBytes    = ByteSize;
			}

			{
				UINT8*        pIndexDataBegin;
				CD3DX12_RANGE readRange( 0, 0 );
				m_IndexBufferResource->GetD3D12Resource()->Map( 0, &readRange, reinterpret_cast<void**>( &pIndexDataBegin ) );
				uint32 ByteSize = m_IndexData.size() * sizeof( uint32 );
				memcpy( pIndexDataBegin, &m_IndexData[0], ByteSize );
				m_IndexBufferResource->GetD3D12Resource()->Unmap( 0, nullptr );

				m_IndexBufferView.BufferLocation = m_IndexBufferResource->GetD3D12Resource()->GetGPUVirtualAddress();
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