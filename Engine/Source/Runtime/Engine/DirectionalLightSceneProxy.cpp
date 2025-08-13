#include "DrnPCH.h"
#include "DirectionalLightSceneProxy.h"
#include "Runtime/Components/DirectionalLightComponent.h"
#include <pix3.h>

#define DIRECTIONAL_SHADOW_SIZE 2048
#define DIRECTIONAL_SHADOW_NEARZ 0.1f
#define DIRECTIONAL_SHADOW_CASCADE_MAX 8

namespace Drn
{
	DirectionalLightSceneProxy::DirectionalLightSceneProxy( class DirectionalLightComponent* InComponent )
		: LightSceneProxy(InComponent)
		, m_DirectionalLightComponent(InComponent)
		, m_ShadowDistance(500.0f)
		, m_CascadeCount(3)
		, m_CascadeLogDistribution(0.65f)
		, m_CascadeDepthScale(1.5f)
		, m_LightBuffer(nullptr)
		, m_ShadowBuffer(nullptr)
		, m_ShadowmapResource(nullptr)
	{
		D3D12_DESCRIPTOR_HEAP_DESC DepthHeapDesc = {};
		DepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DepthHeapDesc.NumDescriptors = DIRECTIONAL_SHADOW_CASCADE_MAX;
		// TODO: make pooled and allocate on demand
		Renderer::Get()->GetD3D12Device()->CreateDescriptorHeap( &DepthHeapDesc, IID_PPV_ARGS(m_DsvHeap.ReleaseAndGetAddressOf()) );
#if D3D12_Debug_INFO
		m_DsvHeap->SetName(StringHelper::s2ws("DsvHeapDirectionalLightShadowmap_" + m_Name).c_str());
#endif

		m_ShadowmapCpuHandles.resize(DIRECTIONAL_SHADOW_CASCADE_MAX);
		for (int32 i = 0; i < DIRECTIONAL_SHADOW_CASCADE_MAX; i++)
		{
			m_ShadowmapCpuHandles[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_DsvHeap->GetCPUDescriptorHandleForHeapStart(), i, Renderer::Get()->GetDsvIncrementSize());;
		}

// --------------------------------------------------------------------------------------------------

		m_LightBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 256 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_LightBuffer->SetName("CB_DircetionalLight_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.BufferLocation = m_LightBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ResourceViewDesc.SizeInBytes = 256;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ResourceViewDesc, m_LightBuffer->GetCpuHandle());
	}

	DirectionalLightSceneProxy::~DirectionalLightSceneProxy()
	{
		ReleaseShadowmap();
		ReleaseBuffers();
	}

	void DirectionalLightSceneProxy::Render( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		// TODO: remove. should not be aware of component. lazy updae
		m_LightData.Direction = m_DirectionalLightComponent->GetWorldRotation().GetVector();
		m_LightData.Color = m_DirectionalLightComponent->GetScaledColor();
		m_LightData.ShadowmapBufferIndex = m_CastShadow ? Renderer::Get()->GetBindlessSrvIndex(m_ShadowBuffer->GetGpuHandle()) : 0;
		
		UINT8* ConstantBufferStart;
		CD3DX12_RANGE readRange( 0, 0 );
		m_LightBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
		memcpy( ConstantBufferStart, &m_LightData, sizeof(DirectionalLightData));
		m_LightBuffer->GetD3D12Resource()->Unmap(0, nullptr);
		
		CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_LightBuffer->GetGpuHandle()), 1);
		
		// TODO: make light flags enum. e. g. 1: Pointlight. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 4;
		CommandList->SetGraphicsRoot32BitConstant(0, LightFlags, 7);
		
		if (m_CastShadow)
		{
			ResourceStateTracker::Get()->TransiationResource(m_ShadowmapResource, D3D12_RESOURCE_STATE_DEPTH_READ);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList);
		}
		
		CommonResources::Get()->m_BackfaceScreenTriangle->BindAndDraw(CommandList);
	}

	void DirectionalLightSceneProxy::RenderShadowDepth( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer )
	{
		if (m_CastShadow && !m_ShadowmapResource)
		{
			AllocateShadowmap(CommandList);
		}

		else if (!m_CastShadow && m_ShadowmapResource)
		{
			ReleaseShadowmap();
		}

		if (m_CastShadow)
		{
			PIXBeginEvent( CommandList, 1, "DirectionalLightShadow");

			CalculateSplitDistance();

			//static int Counter = 0;
			//for (int32 i = 0; i < m_CascadeCount; i++)
			//{
			//	m_CSWorldToProjetcionMatrices.emplace_back(GetShadowSplitBoundsMatrix(Renderer->GetSceneView(), Renderer->GetSceneView().CameraPos, m_SplitDistances[i], m_SplitDistances[i + 1]));
			//
			//	if (Counter == 200)
			//	{
			//
			//		XMMATRIX M = XMMatrixLookAtLH(XMVectorZero(), XMLoadFloat3(m_LightData.Direction.Get()), XMLoadFloat3(Vector::UpVector.Get()));
			//	
			//		XMMATRIX P = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1980.0f / 1080.0f, m_SplitDistances[i], m_SplitDistances[i + 1]);
			//		XMMATRIX W = Renderer->GetSceneView().WorldToView.Get() * P;
			//		W = XMMatrixInverse(NULL, W);
			//	
			//		m_DirectionalLightComponent->GetWorld()->DrawDebugFrustum( W, Color::Red, 0, 50);
			//		m_DirectionalLightComponent->GetWorld()->DrawDebugFrustum( Matrix(XMMatrixInverse( NULL, m_CSWorldToProjetcionMatrices[i].Get())), Color::White, 0, 50);
			//	}
			//}
			//Counter++;

			D3D12_RECT R = CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX );
			CD3DX12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)DIRECTIONAL_SHADOW_SIZE, (float)DIRECTIONAL_SHADOW_SIZE);

			CommandList->RSSetViewports(1, &Viewport);
			CommandList->RSSetScissorRects(1, &R);

			m_ShadowData.DepthBias = m_DepthBias;
			m_ShadowData.InvShadowResolution = 1.0f / DIRECTIONAL_SHADOW_SIZE;
			m_ShadowData.CacadeCount = m_CascadeCount;
			m_ShadowData.ShadowmapTextureIndex = Renderer::Get()->GetBindlessSrvIndex(m_ShadowmapResource->GetGpuHandle());

			for (int32 i = 0; i < m_CascadeCount; i++)
			{
				m_ShadowData.CsWorldToProjectionMatrices[i] = GetShadowSplitBoundsMatrix(Renderer->GetSceneView(), Renderer->GetSceneView().CameraPos, m_SplitDistances[i], m_SplitDistances[i + 1]);
			}

			for (int32 i = 0; i < m_CascadeCount; i++)
			{
				m_ShadowData.SplitDistances[i] = m_SplitDistances[i + 1];
			}

			UINT8* ConstantBufferStart;
			CD3DX12_RANGE readRange( 0, 0 );
			m_ShadowBuffer->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
			memcpy( ConstantBufferStart, &m_ShadowData, sizeof(DirectionalLightShadowData));
			m_ShadowBuffer->GetD3D12Resource()->Unmap(0, nullptr);

			ResourceStateTracker::Get()->TransiationResource(m_ShadowmapResource, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			ResourceStateTracker::Get()->FlushResourceBarriers(CommandList);

			for (int i = 0; i < m_CascadeCount; i++)
			{
				CommandList->ClearDepthStencilView(m_ShadowmapCpuHandles[i], D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
				CommandList->OMSetRenderTargets(0, nullptr, false, &m_ShadowmapCpuHandles[i]);

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

				UINT8* ConstantBufferStart;
				CD3DX12_RANGE readRange( 0, 0 );
				m_CsWorldToProjectionMatricesBuffer[i]->GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>( &ConstantBufferStart ) );
				memcpy( ConstantBufferStart, &m_ShadowData.CsWorldToProjectionMatrices[i], sizeof(float) * 16);
				m_CsWorldToProjectionMatricesBuffer[i]->GetD3D12Resource()->Unmap(0, nullptr);

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------

				CommandList->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
				CommandList->SetGraphicsRoot32BitConstant(0, Renderer::Get()->GetBindlessSrvIndex(m_CsWorldToProjectionMatricesBuffer[i]->GetGpuHandle()), 6);
				
				for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
				{
					Proxy->RenderShadowPass(CommandList, Renderer, this);
				}
			}

			PIXEndEvent(CommandList);
		}

	}

	void DirectionalLightSceneProxy::AllocateShadowmap( ID3D12GraphicsCommandList2* CommandList )
	{
		D3D12_CLEAR_VALUE ShadowmapClearValue = {};
		ShadowmapClearValue.Format = DXGI_FORMAT_D16_UNORM;
		ShadowmapClearValue.DepthStencil.Depth = 1;

		m_ShadowmapResource = Resource::Create(D3D12_HEAP_TYPE_DEFAULT,
			CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16_TYPELESS, DIRECTIONAL_SHADOW_SIZE, DIRECTIONAL_SHADOW_SIZE, m_CascadeCount, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, ShadowmapClearValue);

#if D3D12_Debug_INFO
		// TODO: add component name
		m_ShadowmapResource->SetName("DirectionalLightShadowmap");
#endif

		for (int32 i = 0; i < m_CascadeCount; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
			DepthViewDesc.Format = DXGI_FORMAT_D16_UNORM;
			DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			DepthViewDesc.Texture2DArray.MipSlice = 0;
			DepthViewDesc.Texture2DArray.FirstArraySlice = i;
			DepthViewDesc.Texture2DArray.ArraySize = 1;
			Renderer::Get()->GetD3D12Device()->CreateDepthStencilView( m_ShadowmapResource->GetD3D12Resource(), &DepthViewDesc, m_ShadowmapCpuHandles[i] );
		}

// -----------------------------------------------------------------------------------------------------------

		D3D12_SHADER_RESOURCE_VIEW_DESC ResourceViewDesc = {};
		ResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		ResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		ResourceViewDesc.Format = DXGI_FORMAT_R16_UNORM;
		ResourceViewDesc.Texture2DArray.MipLevels = 1;
		ResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		ResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0;
		ResourceViewDesc.Texture2DArray.ArraySize = m_CascadeCount;
		ResourceViewDesc.Texture2DArray.FirstArraySlice = 0;

		Renderer::Get()->GetD3D12Device()->CreateShaderResourceView(m_ShadowmapResource->GetD3D12Resource(), &ResourceViewDesc, m_ShadowmapResource->GetCpuHandle());

// --------------------------------------------------------------------------------------------------

		m_ShadowBuffer = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer( 1024 ), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
		m_ShadowBuffer->SetName("CB_DirectionalLightShadow_" + m_Name);
#endif

		D3D12_CONSTANT_BUFFER_VIEW_DESC ShadowResourceViewDesc = {};
		ShadowResourceViewDesc.BufferLocation = m_ShadowBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
		ShadowResourceViewDesc.SizeInBytes = 1024;
		Renderer::Get()->GetD3D12Device()->CreateConstantBufferView( &ShadowResourceViewDesc, m_ShadowBuffer->GetCpuHandle());

// --------------------------------------------------------------------------------------------------

		m_CsWorldToProjectionMatricesBuffer.resize(m_CascadeCount);
		for (int32 i = 0; i < m_CascadeCount; i++)
		{
			m_CsWorldToProjectionMatricesBuffer[i] = Resource::Create(D3D12_HEAP_TYPE_UPLOAD, CD3DX12_RESOURCE_DESC::Buffer(256), D3D12_RESOURCE_STATE_GENERIC_READ, false);
#if D3D12_Debug_INFO
			m_CsWorldToProjectionMatricesBuffer[i]->SetName("CB_CsProjectionMatrix_" + std::to_string(i));
#endif

			D3D12_CONSTANT_BUFFER_VIEW_DESC Desc = {};
			Desc.BufferLocation = m_CsWorldToProjectionMatricesBuffer[i]->GetD3D12Resource()->GetGPUVirtualAddress();
			Desc.SizeInBytes = 256;
			Renderer::Get()->GetD3D12Device()->CreateConstantBufferView(&Desc, m_CsWorldToProjectionMatricesBuffer[i]->GetCpuHandle());
		}
	}

	void DirectionalLightSceneProxy::ReleaseShadowmap()
	{
		if (m_ShadowmapResource)
		{
			m_ShadowmapResource->ReleaseBufferedResource();
			m_ShadowmapResource = nullptr;

			if (m_ShadowBuffer)
			{
				m_ShadowBuffer->ReleaseBufferedResource();
				m_ShadowBuffer = nullptr;
			}

			for (int32 i = 0; i < m_CsWorldToProjectionMatricesBuffer.size(); i++)
			{
				m_CsWorldToProjectionMatricesBuffer[i]->ReleaseBufferedResource();
			}
			m_CsWorldToProjectionMatricesBuffer.clear();
		}
	}

	void DirectionalLightSceneProxy::ReleaseBuffers()
	{
		if (m_LightBuffer)
		{
			m_LightBuffer->ReleaseBufferedResource();
			m_LightBuffer = nullptr;
		}
	}

	void DirectionalLightSceneProxy::UpdateResources( ID3D12GraphicsCommandList2* CommandList )
	{
		if (m_LightComponent && m_LightComponent->IsRenderStateDirty())
		{
			m_LightComponent->ClearRenderStateDirty();
			m_LightColor = m_DirectionalLightComponent->GetScaledColor();
			m_CastShadow = m_DirectionalLightComponent->IsCastingShadow();

			m_Direction = m_DirectionalLightComponent->GetWorldRotation().GetVector();
			m_ShadowDistance = m_DirectionalLightComponent->GetShadowDistance();
			m_CascadeCount = m_DirectionalLightComponent->GetCascadeCount();
			m_CascadeLogDistribution = m_DirectionalLightComponent->GetCascadeLogDistribution();
			m_CascadeDepthScale = m_DirectionalLightComponent->GetCascadeDepthScale();
			m_DepthBias = m_DirectionalLightComponent->GetDepthBias();

			CalculateSplitDistance();

			if (m_CastShadow && (!m_ShadowmapResource || m_ShadowmapResource->GetD3D12Resource()->GetDesc().DepthOrArraySize != m_CascadeCount))
			{
				ReleaseShadowmap();
				AllocateShadowmap(CommandList);
			}

			else if (!m_CastShadow && m_ShadowmapResource)
			{
				ReleaseShadowmap();
			}
		}
	}

	void DirectionalLightSceneProxy::CalculateSplitDistance()
	{
		m_SplitDistances.clear();

		for (int32 i = 0; i < m_CascadeCount + 1; i++)
		{
			// see https://computergraphics.stackexchange.com/questions/13026/cascaded-shadow-mapping-csm-partitioning-the-frustum-to-a-nearly-1-by-1-mappi

			float LogDistance = DIRECTIONAL_SHADOW_NEARZ * pow((m_ShadowDistance / DIRECTIONAL_SHADOW_NEARZ), (float)i / m_CascadeCount);
			float UniformDistance = DIRECTIONAL_SHADOW_NEARZ + (m_ShadowDistance - DIRECTIONAL_SHADOW_NEARZ) * ( (float)i / m_CascadeCount);

			float DistanceFactor = std::lerp( UniformDistance, LogDistance, m_CascadeLogDistribution);
			m_SplitDistances.push_back(DistanceFactor);
		}
	}

	Matrix DirectionalLightSceneProxy::GetShadowSplitBoundsMatrix( const SceneRendererView& View,
		const Vector& ViewOrigin, float SplitNear, float SplitFar )
	{
		const Matrix& ViewMatrix = View.WorldToView;
		const Matrix& ProjectionMatrix = View.ViewToProjection;

		const Vector& CameraDirection = View.CameraDir;
		const Vector& LightDirection = m_Direction * -1;

		float AspectRatio = ProjectionMatrix.m_Matrix.m[1][1] / ProjectionMatrix.m_Matrix.m[0][0];
		bool IsPerspective = true;

		float HalfHorizontalFOV = IsPerspective ? std::atan(1.0f / ProjectionMatrix.m_Matrix.m[0][0]) : XM_PIDIV4;
		float HalfVerticalFOV = IsPerspective ? std::atan(1.0f / ProjectionMatrix.m_Matrix.m[1][1]) : std::atan((std::tan(XM_PIDIV4) / AspectRatio));
		float AsymmetricFOVScaleX = ProjectionMatrix.m_Matrix.m[2][0];
		float AsymmetricFOVScaleY = ProjectionMatrix.m_Matrix.m[2][1];
		
		const float StartHorizontalTotalLength = SplitNear * std::tan(HalfHorizontalFOV);
		const float StartVerticalTotalLength = SplitNear * std::tan(HalfVerticalFOV);
		const Vector StartCameraLeftOffset = ViewMatrix.GetColumn(0) * -StartHorizontalTotalLength * (1 + AsymmetricFOVScaleX);
		const Vector StartCameraRightOffset = ViewMatrix.GetColumn(0) *  StartHorizontalTotalLength * (1 - AsymmetricFOVScaleX);
		const Vector StartCameraBottomOffset = ViewMatrix.GetColumn(1) * -StartVerticalTotalLength * (1 + AsymmetricFOVScaleY);
		const Vector StartCameraTopOffset = ViewMatrix.GetColumn(1) *  StartVerticalTotalLength * (1 - AsymmetricFOVScaleY);
		
		const float EndHorizontalTotalLength = SplitFar * std::tan(HalfHorizontalFOV);
		const float EndVerticalTotalLength = SplitFar * std::tan(HalfVerticalFOV);
		const Vector EndCameraLeftOffset = ViewMatrix.GetColumn(0) * -EndHorizontalTotalLength * (1 + AsymmetricFOVScaleX);
		const Vector EndCameraRightOffset = ViewMatrix.GetColumn(0) *  EndHorizontalTotalLength * (1 - AsymmetricFOVScaleX);
		const Vector EndCameraBottomOffset = ViewMatrix.GetColumn(1) * -EndVerticalTotalLength * (1 + AsymmetricFOVScaleY);
		const Vector EndCameraTopOffset = ViewMatrix.GetColumn(1) *  EndVerticalTotalLength * (1 - AsymmetricFOVScaleY);

		Vector CascadeFrustumVerts[8];
		CascadeFrustumVerts[0] = ViewOrigin + CameraDirection * SplitNear + StartCameraRightOffset + StartCameraTopOffset;    // 0 Near Top    Right
		CascadeFrustumVerts[1] = ViewOrigin + CameraDirection * SplitNear + StartCameraRightOffset + StartCameraBottomOffset; // 1 Near Bottom Right
		CascadeFrustumVerts[2] = ViewOrigin + CameraDirection * SplitNear + StartCameraLeftOffset + StartCameraTopOffset;     // 2 Near Top    Left
		CascadeFrustumVerts[3] = ViewOrigin + CameraDirection * SplitNear + StartCameraLeftOffset + StartCameraBottomOffset;  // 3 Near Bottom Left
		CascadeFrustumVerts[4] = ViewOrigin + CameraDirection * SplitFar + EndCameraRightOffset + EndCameraTopOffset;       // 4 Far  Top    Right
		CascadeFrustumVerts[5] = ViewOrigin + CameraDirection * SplitFar + EndCameraRightOffset + EndCameraBottomOffset;    // 5 Far  Bottom Right
		CascadeFrustumVerts[6] = ViewOrigin + CameraDirection * SplitFar + EndCameraLeftOffset + EndCameraTopOffset;       // 6 Far  Top    Left
		CascadeFrustumVerts[7] = ViewOrigin + CameraDirection * SplitFar + EndCameraLeftOffset + EndCameraBottomOffset;    // 7 Far  Bottom Left
		
		//float TanHalfFOVx = std::tan(HalfHorizontalFOV);
		//float TanHalfFOVy = std::tan(HalfVerticalFOV);
		//float FrustumLength = SplitFar - SplitNear;
		//
		//float FarX = TanHalfFOVx * SplitFar;
		//float FarY = TanHalfFOVy * SplitFar;
		//float DiagonalASq = FarX * FarX + FarY * FarY;
		//
		//float NearX = TanHalfFOVx * SplitNear;
		//float NearY = TanHalfFOVy * SplitNear;
		//float DiagonalBSq = NearX * NearX + NearY * NearY;
		//
		//float OptimalOffset = (DiagonalBSq - DiagonalASq) / (2.0f * FrustumLength) + FrustumLength * 0.5f;
		//float CentreZ = SplitFar - OptimalOffset - SplitNear;
		//CentreZ = std::clamp( CentreZ, SplitNear, SplitFar );
		//Sphere CascadeSphere(ViewOrigin + CameraDirection * CentreZ, 0);
		//for (int32 Index = 0; Index < 8; Index++)
		//{
		//	CascadeSphere.Radius = std::max(CascadeSphere.Radius, Vector::DistSquared(CascadeFrustumVerts[Index], CascadeSphere.Center));
		//}
		//
		//CascadeSphere.Radius = std::max(std::sqrt(CascadeSphere.Radius), 1.0f);
		//return CascadeSphere;

// ------------------------------------------------------------------------------------------------------------------

		Matrix LightViewMatrix = XMMatrixLookAtLH(XMVectorZero(), XMLoadFloat3(m_Direction.Get()), XMLoadFloat3(Vector::UpVector.Get()));

		float MinX, MinY, MinZ = FLT_MAX;
		float MaxX, MaxY, MaxZ = FLT_MIN;

		MinX = MinY = MinZ = FLT_MAX;
		MaxX = MaxY = MaxZ = FLT_MIN;

		for (int32 i = 0; i < 8; i++)
		{
			Vector4 CameraPos = LightViewMatrix.TransformVector4(Vector4(CascadeFrustumVerts[i], 1));
			Vector CameraWorldPos = Vector(CameraPos.GetX(), CameraPos.GetY(), CameraPos.GetZ()) / CameraPos.GetW();
			
			MinX = std::min(MinX, CameraWorldPos.GetX());
			MinY = std::min(MinY, CameraWorldPos.GetY());
			MinZ = std::min(MinZ, CameraWorldPos.GetZ());

			MaxX = std::max(MaxX, CameraWorldPos.GetX());
			MaxY = std::max(MaxY, CameraWorldPos.GetY());
			MaxZ = std::max(MaxZ, CameraWorldPos.GetZ());
		}

		Vector MinPos = Vector(MinX, MinY, MinZ);
		Vector MaxPos = Vector(MaxX, MaxY, MaxZ);

		float Width = std::max(1.0f, MaxX - MinX);
		float Height = std::max(1.0f, MaxY - MinY);
		float Depth = std::max(1.0f, MaxZ - MinZ);

		float ScaledDepth = Depth * m_CascadeDepthScale;

		Vector CameraPosCenter = Vector((MinX + MaxX) / 2, (MinY + MaxY) / 2, MinZ - (ScaledDepth - Depth) / 2);
		Matrix LightViewInverseMatrix = XMMatrixInverse(NULL, LightViewMatrix.Get());
		Vector CameraPos = LightViewInverseMatrix.TransformVector(CameraPosCenter);

		XMMATRIX V = XMMatrixLookAtLH(XMLoadFloat3(CameraPos.Get()), XMLoadFloat3( (CameraPos + m_Direction).Get() ), XMLoadFloat3(Vector::UpVector.Get()));
		XMMATRIX P = XMMatrixOrthographicLH( Width, Height, 0, ScaledDepth);

		Matrix Result = V * P;
		return Result;
	}

#if WITH_EDITOR
	void DirectionalLightSceneProxy::DrawAttenuation( World* InWorld )
	{
		InWorld->DrawDebugArrow(m_DirectionalLightComponent->GetWorldLocation(),
			m_DirectionalLightComponent->GetWorldLocation() + m_DirectionalLightComponent->GetWorldRotation().GetVector() * 1.2f, 0.1f, Color::White, 0.0f, 0.0f);
	}
#endif
}