#include "DrnPCH.h"
#include "DebugViewPhysics.h"

#include "Renderer.h"

using namespace Microsoft::WRL;

namespace Drn
{
	DebugViewPhysics::DebugViewPhysics()
	{
		
	}

	void DebugViewPhysics::Init( SceneRenderer* InSceneRenderer, dx12lib::CommandList* CommandList )
	{
		m_SceneRenderer = InSceneRenderer;
		dx12lib::Device* Device = Renderer::Get()->GetDevice();

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		ComPtr<ID3DBlob> vertexShaderBlob;
		ThrowIfFailed( D3DReadFileToBlob( L"TestShader_VS.cso", &vertexShaderBlob ) );

		ComPtr<ID3DBlob> pixelShaderBlob;
		ThrowIfFailed( D3DReadFileToBlob( L"TestShader_PS.cso", &pixelShaderBlob ) );

		DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

		// DXGI_SAMPLE_DESC sampleDesc = pDevice->GetMultisampleQualityLevels( backBufferFormat );

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count            = 1;

		D3D12_RT_FORMAT_ARRAY rtvFormats = {};
		rtvFormats.NumRenderTargets      = 1;
		rtvFormats.RTFormats[0]          = backBufferFormat;

		struct PipelineStateStream
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
			CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
			CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
			CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
		} pipelineStateStream;

		pipelineStateStream.pRootSignature        = m_SceneRenderer->m_RootSignature->GetD3D12RootSignature().Get();
		pipelineStateStream.InputLayout           = { inputLayout, _countof( inputLayout ) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
		pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
		pipelineStateStream.DSVFormat             = depthBufferFormat;
		pipelineStateStream.RTVFormats            = rtvFormats;
		pipelineStateStream.SampleDesc            = sampleDesc;

		m_CollisionPSO = Device->CreatePipelineStateObject(pipelineStateStream);
	}

	void DebugViewPhysics::Shutdown()
	{
		m_CollisionPSO.reset();
	}

	void DebugViewPhysics::RenderCollisions( dx12lib::CommandList* CommandList )
	{
		CommandList->SetPipelineState(m_CollisionPSO);
		CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );

		PhysicScene* PScene = m_SceneRenderer->GetScene()->GetWorld()->GetPhysicScene();

		if (PScene)
		{
			PxActorTypeFlags Flags = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
	
			uint32 NumActors = PScene->GetPhysxScene()->getNbActors(Flags);
			PxActor** Actors = new PxActor*[NumActors];

			uint32 ActorsNum = PScene->GetPhysxScene()->getActors(Flags, Actors, NumActors);

			for (int32 i = 0; i < ActorsNum; i++)
			{
				PxActor* Actor = Actors[i];
				
				if ( Actor && Actor->getOwnerClient() == PX_DEFAULT_CLIENT )
				{
					PxRigidActor* RigidActor = Actor->is<PxRigidActor>();

					if (RigidActor)
					{
						Transform RigidTransform = P2Transform(RigidActor->getGlobalPose());
						//LOG( LogSceneRenderer, Info, "%s", RigidTransform.ToString().c_str() );
					}
				}
			}

			delete Actors;
		}
	}

	void DebugViewPhysics::RenderPhysxDebug( dx12lib::CommandList * CommandList )
	{
		const PxRenderBuffer& Rb = m_SceneRenderer->GetScene()->GetWorld()->GetPhysicScene()->GetPhysxScene()->getRenderBuffer();
		uint32 NumLines = Rb.getNbLines();

		CollisionVertexData.clear();
		CollisionVertexData.reserve(NumLines);

		CollisionIndexData.clear();
		CollisionIndexData.reserve(NumLines);

		StaticMeshVertexBuffer VertexElement;
		Vector Color;

		for ( PxU32 i = 0; i < NumLines; i++ )
		{
			const PxDebugLine& line = Rb.getLines()[i];
				
			VertexElement.Pos_X = line.pos0.x;
			VertexElement.Pos_Y = line.pos0.y;
			VertexElement.Pos_Z = line.pos0.z;

			Color = Vector::FromU32(line.color0);

			VertexElement.Color_R = Color.GetX();
			VertexElement.Color_G = Color.GetY();
			VertexElement.Color_B = Color.GetZ();

			CollisionVertexData.push_back(VertexElement);

			VertexElement.Pos_X = line.pos1.x;
			VertexElement.Pos_Y = line.pos1.y;
			VertexElement.Pos_Z = line.pos1.z;

			VertexElement.Color_R = Color.GetX();
			VertexElement.Color_G = Color.GetY();
			VertexElement.Color_B = Color.GetZ();

			CollisionVertexData.push_back(VertexElement);

			CollisionIndexData.push_back( i * 2 );
			CollisionIndexData.push_back( i * 2 + 1 );
		}

		if (CollisionVertexData.empty())
		{
			return;
		}

		CommandList->SetPipelineState(m_CollisionPSO);
		CommandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINELIST );

		CollisionVertexBuffer = CommandList->CopyVertexBuffer( CollisionVertexData.size(), sizeof(StaticMeshVertexBuffer), CollisionVertexData.data());
		CollisionIndexBuffer = CommandList->CopyIndexBuffer( CollisionIndexData.size(), DXGI_FORMAT_R32_UINT, CollisionIndexData.data());

		XMMATRIX modelMatrix = Matrix().Get();

		auto viewport = m_SceneRenderer->m_RenderTarget.GetViewport();
		float    aspectRatio = viewport.Width / viewport.Height;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		m_SceneRenderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);
		
		XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
		mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

		CommandList->SetGraphics32BitConstants( 0, mvpMatrix );

		CommandList->SetVertexBuffer( 0, CollisionVertexBuffer );
		CommandList->SetIndexBuffer( CollisionIndexBuffer );
		CommandList->DrawIndexed( CollisionIndexBuffer->GetNumIndices());
	}
}