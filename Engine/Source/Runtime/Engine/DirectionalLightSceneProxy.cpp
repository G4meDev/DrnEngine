#include "DrnPCH.h"
#include "DirectionalLightSceneProxy.h"
#include "Runtime/Components/DirectionalLightComponent.h"
#include "Runtime/Renderer/RenderTexture.h"
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
		, m_ShadowmapResource(nullptr)
	{}

	DirectionalLightSceneProxy::~DirectionalLightSceneProxy()
	{
		ReleaseShadowmap();
		ReleaseBuffers();
	}

	void DirectionalLightSceneProxy::Render( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		// TODO: remove. should not be aware of component. lazy update
		m_LightData.Direction = m_DirectionalLightComponent->GetWorldRotation().GetVector();
		m_LightData.Color = m_DirectionalLightComponent->GetScaledColor();
		m_LightData.ShadowmapBufferIndex = m_CastShadow ? ShadowBuffer->GetViewIndex() : 0;
		
		TRefCountPtr<RenderUniformBuffer> LightBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(DirectionalLightData), EUniformBufferUsage::SingleFrame, &m_LightData);

		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, LightBuffer->GetViewIndex(), 1);
		
		// TODO: make light flags enum. e. g. 1: Point light. 2: Spotlight. 3: RectLight. 4: Dynamic. ...
		uint32 LightFlags = 4;
		CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, LightFlags, 7);
		
		if (m_CastShadow)
		{
			CommandList->TransitionResourceWithTracking(m_ShadowmapResource->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			CommandList->FlushBarriers();
		}
		
		CommonResources::Get()->m_BackfaceScreenTriangle->BindAndDraw(CommandList);
	}

	void DirectionalLightSceneProxy::RenderShadowDepth( D3D12CommandList* CommandList, SceneRenderer* Renderer )
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
			PIXBeginEvent( CommandList->GetD3D12CommandList(), 1, "DirectionalLightShadow");

			CalculateSplitDistance();

			CommandList->SetViewport( 0, 0, 0, DIRECTIONAL_SHADOW_SIZE, DIRECTIONAL_SHADOW_SIZE, 1 );

			m_ShadowData.DepthBias = m_DepthBias;
			m_ShadowData.InvShadowResolution = 1.0f / DIRECTIONAL_SHADOW_SIZE;
			m_ShadowData.CacadeCount = m_CascadeCount;
			m_ShadowData.ShadowmapTextureIndex = m_ShadowmapResource->GetShaderResourceView()->GetDescriptorHeapIndex();

			for (int32 i = 0; i < m_CascadeCount; i++)
			{
				m_ShadowData.CsWorldToProjectionMatrices[i] = GetShadowSplitBoundsMatrix(Renderer->GetSceneView(), Renderer->GetSceneView().CameraPos, m_SplitDistances[i], m_SplitDistances[i + 1]);
			}

			for (int32 i = 0; i < m_CascadeCount; i++)
			{
				m_ShadowData.SplitDistances[i] = m_SplitDistances[i + 1];
			}

			ShadowBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(DirectionalLightShadowData), EUniformBufferUsage::SingleFrame, &m_ShadowData);

			CommandList->TransitionResourceWithTracking(m_ShadowmapResource->GetResource(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
			CommandList->FlushBarriers();

			for (int i = 0; i < m_CascadeCount; i++)
			{
				CommandList->ClearDepthTexture( m_ShadowmapViews[i], true, 1, false, 0);

				D3D12_CPU_DESCRIPTOR_HANDLE Handle = m_ShadowmapViews[i]->GetView();
				CommandList->GetD3D12CommandList()->OMSetRenderTargets(0, nullptr, false, &Handle);

				TRefCountPtr<RenderUniformBuffer> CsWorldToProjectionMatricesBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(Matrix), EUniformBufferUsage::SingleFrame, &m_ShadowData.CsWorldToProjectionMatrices[i]);

				CommandList->GetD3D12CommandList()->SetGraphicsRootSignature(Renderer::Get()->m_BindlessRootSinature.Get());
				CommandList->GetD3D12CommandList()->SetGraphicsRoot32BitConstant(0, CsWorldToProjectionMatricesBuffer->GetViewIndex(), 6);
				
				for (PrimitiveSceneProxy* Proxy : Renderer->GetScene()->GetPrimitiveProxies())
				{
					Proxy->RenderShadowPass(CommandList, Renderer, this);
				}
			}

			PIXEndEvent(CommandList->GetD3D12CommandList());
		}

	}

	void DirectionalLightSceneProxy::AllocateShadowmap( D3D12CommandList* CommandList )
	{
		RenderResourceCreateInfo ShadowmapCreateInfo( nullptr, nullptr, ClearValueBinding::DepthOne, "DirectionalLightShadowmap" );
		m_ShadowmapResource = RenderTexture2DArray::Create(Renderer::Get()->GetCommandList_Temp(), DIRECTIONAL_SHADOW_SIZE, DIRECTIONAL_SHADOW_SIZE, m_CascadeCount, DXGI_FORMAT_D16_UNORM, 1, 1, true,
			(ETextureCreateFlags)(ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource), ShadowmapCreateInfo);

		m_ShadowmapViews.clear();
		for (int32 i = 0; i < m_CascadeCount; i++)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC DepthViewDesc = {};
			DepthViewDesc.Format = DXGI_FORMAT_D16_UNORM;
			DepthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			DepthViewDesc.Texture2DArray.MipSlice = 0;
			DepthViewDesc.Texture2DArray.FirstArraySlice = i;
			DepthViewDesc.Texture2DArray.ArraySize = 1;

			m_ShadowmapViews.push_back(new DepthStencilView(CommandList->GetParentDevice(), DepthViewDesc, m_ShadowmapResource->m_ResourceLocation, false));
		}

	}

	void DirectionalLightSceneProxy::ReleaseShadowmap()
	{
		m_ShadowmapResource = nullptr;
		ShadowBuffer = nullptr;
	}

	void DirectionalLightSceneProxy::ReleaseBuffers()
	{}

	void DirectionalLightSceneProxy::UpdateResources( D3D12CommandList* CommandList )
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

			if (m_CastShadow && (!m_ShadowmapResource || m_ShadowmapResource->GetResource()->GetDesc().DepthOrArraySize != m_CascadeCount))
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

}