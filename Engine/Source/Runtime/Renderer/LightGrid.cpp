#include "DrnPCH.h"
#include "LightGrid.h"

#include "Runtime/Engine/DirectionalLightSceneProxy.h"
#include "Runtime/Engine/PointLightSceneProxy.h"
#include "Runtime/Engine/SpotLightSceneProxy.h"
#include "Runtime/Engine/SkyLightSceneProxy.h"
#include "Runtime/Engine/ReflectionCaptureProxy.h"

#include "Runtime/Renderer/RenderBuffer/ReflectionEnvironmentBuffer.h"

namespace Drn
{
	Vector GetLightGridZParams(float NearPlane, float FarPlane)
	{
		//double NearOffset = .095 * 100;
		double NearOffset = .095;
		double S = 4.05;

		double N = NearPlane + NearOffset;
		double F = FarPlane;

		double O = (F - N * std::exp2((LIGHT_GRID_SIZE_Z - 1) / S)) / (F - N);
		double B = (1 - O) / N;

		return Vector(B, O, S);
	}

	float GetTanRadAngleOrZero(float coneAngle)
	{
		if (coneAngle < XM_PIDIV2)
		{
			return std::tan(coneAngle);
		}

		return 0.0f;
	}

	void LightGrid::ComputeLightGrid()
	{
		D3D12CommandList* CmdList = View->GetCommandList();

		Data.HasDirectionalLight = 0;
		Data.NumCulledLights = 0;
		LocalLightData.clear();
		ViewSpacePosAndRadiusData.clear();
		ViewSpaceDirAndPreprocAngleData.clear();

		ReflectionCaptureData.clear();

		float FurthestLight = 1000;
		float FurthestReflectionCapture = 1000;

		for (BitArray::ConstSetBitIterator It(View->GetVisibleLights()); It; ++It)
		{
			LightSceneProxy* Proxy = View->GetScene()->GetLightProxies()[It.GetIndex()];

			if (Proxy->GetLightType() == ELightType::PointLight)
			{
				PointLightSceneProxy* PointLight = static_cast<PointLightSceneProxy*>(Proxy);
				uint32 LightType = LIGHT_GRID_LIGHT_TYPE_POINT;

				LocalLightData.push_back({});
				LocalLightData.back().LightPositionAndInvRadius = Vector4(PointLight->GetWorldPosition(), 1.0f / PointLight->GetRadius());
				LocalLightData.back().LightColorAndFalloffExponent = Vector4(PointLight->GetColor(), 0.0f);
				LocalLightData.back().LightDirectionAndLightType = Vector4(Vector::ZeroVector, *((float*)&LightType));
				LocalLightData.back().SpotAnglesAndSourceRadiusPacked = Vector4(0.0f, 0.0f, PointLight->GetRadius(), 0.0f);

				ViewSpacePosAndRadiusData.push_back(Vector4(View->GetSceneView().WorldToView.TransformPosition(PointLight->GetWorldPosition()), PointLight->GetRadius()));
				ViewSpaceDirAndPreprocAngleData.push_back(Vector4(0, 0, 0, 0));

				Sphere Bound = PointLight->GetBoundingSphere();
				float Distance = View->GetSceneView().WorldToView.TransformPosition(Bound.Center).GetZ() + Bound.Radius;
				FurthestLight = std::max(FurthestLight, Distance);
			}

			else if (Proxy->GetLightType() == ELightType::SpotLight)
			{
				SpotLightSceneProxy* SpotLight = static_cast<SpotLightSceneProxy*>(Proxy);
				uint32 LightType = LIGHT_GRID_LIGHT_TYPE_SPOT;

				LocalLightData.push_back({});
				LocalLightData.back().LightPositionAndInvRadius = Vector4(SpotLight->GetWorldPosition(), 1.0f / SpotLight->GetAttenuation());
				LocalLightData.back().LightColorAndFalloffExponent = Vector4(SpotLight->GetColor(), 0.0f);
				LocalLightData.back().LightDirectionAndLightType = Vector4(SpotLight->GetDirection(), *((float*)&LightType));
				LocalLightData.back().SpotAnglesAndSourceRadiusPacked = Vector4(SpotLight->GetCosOuterCone(), SpotLight->GetInvCosConeDifference(), SpotLight->GetAttenuation(), 0.0f);

				ViewSpacePosAndRadiusData.push_back(Vector4(View->GetSceneView().WorldToView.TransformPosition(SpotLight->GetWorldPosition()), SpotLight->GetAttenuation()));
				ViewSpaceDirAndPreprocAngleData.push_back((Vector4(View->GetSceneView().WorldToView.TransformVector(SpotLight->GetDirection()), GetTanRadAngleOrZero(SpotLight->GetOuterRadius()))));

				Sphere Bound = SpotLight->GetBoundingSphere();
				float Distance = View->GetSceneView().WorldToView.TransformPosition(Bound.Center).GetZ() + Bound.Radius;
				FurthestLight = std::max(FurthestLight, Distance);
			}

			else if (Proxy->GetLightType() == ELightType::DirectionalLight)
			{
				const float LightIntensitySq = Proxy->GetColor().SizeSquared();
				if (LightIntensitySq > SMALL_NUMBER)
				{
					DirectionalLightSceneProxy* DirectionalLight = static_cast<DirectionalLightSceneProxy*>(Proxy);

					Data.HasDirectionalLight = 1;
					Data.DirectionalLightColor = DirectionalLight->GetColor();
					Data.DirectionalLightDirection = DirectionalLight->GetLightDirection();
				}
			}
		}

		{
			View->m_ReflectionEnvironmentBuffer->GenerateSkycubemap(CmdList, View);

			Data.HasSkyLight = View->m_ReflectionEnvironmentBuffer->GeneratedCubemap.IsValid();
			Data.PreintegeratedGFImageIndex = CommonResources::Get()->m_PreintegratedGF->GetShaderResourceView()->GetDescriptorHeapIndex();
			if (Data.HasSkyLight)
			{
				Data.SkyLightConvolutionIndex = View->m_ReflectionEnvironmentBuffer->GeneratedCubemap->GetShaderResourceView()->GetDescriptorHeapIndex();
				SkyLightSceneProxy* SkyProxy = View->GetScene()->GetSkyLightProxy();
				Data.SkyLightColor = SkyProxy ? SkyProxy->GetColor() : Vector::ZeroVector;
				Data.SkyLightIrradianceIndex = View->m_ReflectionEnvironmentBuffer->GeneratedCubemapIradiance->GetShaderResourceView()->GetDescriptorHeapIndex();
				Data.SkyLightMipCount = View->m_ReflectionEnvironmentBuffer->GeneratedCubemap->GetNumMips();
			}

			{
				SCOPE_STAT("ReflectionCaptureSort");

				std::vector<ReflectionCaptureProxy*> VisibleReflectionProxies;
				for (ReflectionCaptureProxy* Proxy : View->GetScene()->GetReflectionCaptureProxies())
				{
					drn_check(Proxy);

					if (Proxy->HasValidCubemap() && View->ViewFrustum.Contains(Sphere(Proxy->Position, Proxy->InfluenceRadius)))
					{
						VisibleReflectionProxies.push_back(Proxy);
					}
				}

				std::sort(VisibleReflectionProxies.begin(), VisibleReflectionProxies.end(),
				[](const ReflectionCaptureProxy* A, const ReflectionCaptureProxy* B)
				{
					return A->InfluenceRadius < B->InfluenceRadius;
				});

				for (ReflectionCaptureProxy* Proxy : VisibleReflectionProxies)
				{
					uint32 CubemapIndex = Proxy->GetCubemap()->GetShaderResourceView()->GetDescriptorHeapIndex();

					ReflectionCaptureData.push_back({});
					ReflectionCaptureData.back().PositionRadius = Vector4(Proxy->GetPosition(), Proxy->GetRadius());
					ReflectionCaptureData.back().CaptureOffsetBrightness = Vector4(Proxy->GetCaptureOffset(), Proxy->GetBrightness());
					ReflectionCaptureData.back().CubemapIndex = Vector4(*((float*)&CubemapIndex), 0, 0, 0);

					Sphere Bound = Sphere(Proxy->Position, Proxy->InfluenceRadius);
					float Distance = View->GetSceneView().WorldToView.TransformPosition(Bound.Center).GetZ() + Bound.Radius;
					FurthestReflectionCapture = std::max(FurthestLight, Distance);
				}
			}
		}

		uint32 ActualNumLights = LocalLightData.size();
		drn_check(ActualNumLights < LIGHT_GRID_MAX_LOCAL_LIGHTS); // TODO: calculate tight fit for 65kb constant buffer target and clamp extra ones

		if (ActualNumLights == 0)
			LocalLightData.push_back({});
		if (ViewSpacePosAndRadiusData.empty())
			ViewSpacePosAndRadiusData.push_back({});
		if (ViewSpaceDirAndPreprocAngleData.empty())
			ViewSpaceDirAndPreprocAngleData.push_back({});

		uint32 NumReflectionCaptures = ReflectionCaptureData.size();
		if (ReflectionCaptureData.empty())
			ReflectionCaptureData.push_back({});

		IntPoint ViewportSize = View->GetViewportSize();
		IntVector LightGridSize = IntVector(Math::DivideAndRoundUp(ViewportSize.X, LIGHT_GRID_PIXEL_SIZE), Math::DivideAndRoundUp(ViewportSize.Y, LIGHT_GRID_PIXEL_SIZE), LIGHT_GRID_SIZE_Z);
		uint32 NumGridCells = LightGridSize.GetX() * LightGridSize.GetY() * LightGridSize.GetZ();

		LocalLightBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridLocalLightData) * LocalLightData.size(), EUniformBufferUsage::SingleFrame, LocalLightData.data());
		ViewSpacePosAndRadiusBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(Vector4) * ViewSpacePosAndRadiusData.size(), EUniformBufferUsage::SingleFrame, ViewSpacePosAndRadiusData.data());
		ViewSpaceDirAndPreprocAngleBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(Vector4) * ViewSpaceDirAndPreprocAngleData.size(), EUniformBufferUsage::SingleFrame, ViewSpaceDirAndPreprocAngleData.data());

		ReflectionCaptureBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridReflectionCaptureData) * ReflectionCaptureData.size(), EUniformBufferUsage::SingleFrame, ReflectionCaptureData.data());

		if (bDirtyScreenSize)
		{
			{
				RenderResourceCreateInfo BufferInfo("RWNextCulledLightLinkBuffer");
				uint32 Flags = (uint32)EBufferUsageFlags::UnorderedAccess | (uint32)EBufferUsageFlags::ShaderResource;
				RWNextCulledLightLinkBuffer = RenderRawBuffer::Create(CmdList->GetParentDevice(), CmdList, sizeof(uint32),
					1, DXGI_FORMAT_R32_UINT, Flags, D3D12_RESOURCE_STATE_COMMON, true, BufferInfo);
			}

			{
				RenderResourceCreateInfo BufferInfo("RWCulledLightLinkBuffer");
				uint32 Flags = (uint32)EBufferUsageFlags::UnorderedAccess | (uint32)EBufferUsageFlags::ShaderResource;
				RWCulledLightLinkBuffer = RenderRawBuffer::Create(CmdList->GetParentDevice(), CmdList, sizeof(uint32),
					NumGridCells * LIGHT_GRID_MAX_CULLED_LIGHT_PER_CELL * LIGHT_GRID_LIGHT_LINK_STRIDE * LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES , DXGI_FORMAT_R32_UINT, Flags, D3D12_RESOURCE_STATE_COMMON, true, BufferInfo);
			}

			{
				RenderResourceCreateInfo BufferInfo("RWStartGridOffsetBuffer");
				uint32 Flags = (uint32)EBufferUsageFlags::UnorderedAccess | (uint32)EBufferUsageFlags::ShaderResource;
				RWStartGridOffsetBuffer = RenderRawBuffer::Create(CmdList->GetParentDevice(), CmdList, sizeof(uint32),
					NumGridCells * LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES , DXGI_FORMAT_R32_UINT, Flags, D3D12_RESOURCE_STATE_COMMON, true, BufferInfo);
			}

			{
				RenderResourceCreateInfo BufferInfo("RWNextCulledLightDataBuffer");
				uint32 Flags = (uint32)EBufferUsageFlags::UnorderedAccess | (uint32)EBufferUsageFlags::ShaderResource;
				RWNextCulledLightDataBuffer = RenderRawBuffer::Create(CmdList->GetParentDevice(), CmdList, sizeof(uint32),
					1, DXGI_FORMAT_R32_UINT, Flags, D3D12_RESOURCE_STATE_COMMON, true, BufferInfo);
			}

			{
				RenderResourceCreateInfo BufferInfo("RWLightGridNumOffsetBuffer");
				uint32 Flags = (uint32)EBufferUsageFlags::UnorderedAccess | (uint32)EBufferUsageFlags::ShaderResource;
				RWLightGridNumOffsetBuffer = RenderRawBuffer::Create(CmdList->GetParentDevice(), CmdList, 4,
					NumGridCells * LIGHT_GRID_LIGHT_LINK_STRIDE * LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES, DXGI_FORMAT_R32_UINT, Flags, D3D12_RESOURCE_STATE_COMMON, true, BufferInfo);
			}

			{
				RenderResourceCreateInfo BufferInfo("RWLightGridLinkListBuffer");
				uint32 Flags = (uint32)EBufferUsageFlags::UnorderedAccess | (uint32)EBufferUsageFlags::ShaderResource;
				RWLightGridLinkListBuffer = RenderRawBuffer::Create(CmdList->GetParentDevice(), CmdList, LIGHT_GRID_INDEX_TYPE_SIZE,
					NumGridCells * LIGHT_GRID_MAX_CULLED_LIGHT_PER_CELL * LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES, LIGHT_GRID_INDEX_TYPE_SIZE == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, Flags, D3D12_RESOURCE_STATE_COMMON, true, BufferInfo);
			}

			bDirtyScreenSize = false;
		}

		Data.LocalLightBufferIndex = LocalLightBuffer->GetViewIndex();
		Data.ViewSpacePositionAndRadiusIndex = ViewSpacePosAndRadiusBuffer->GetViewIndex();
		Data.LightViewSpaceDirAndPreprocAngleIndex = ViewSpaceDirAndPreprocAngleBuffer->GetViewIndex();
		Data.NumCulledLights = ActualNumLights;
		Data.CulledGridSize = LightGridSize;
		Data.NumGridCells = NumGridCells;
		Data.MaxCulledLightsPerCell = LIGHT_GRID_MAX_CULLED_LIGHT_PER_CELL;
		Data.LightGridPixelSizeShift = std::_Floor_of_log_2(LIGHT_GRID_PIXEL_SIZE);
		
		ViewInfo VInfo;
		View->GetViewInfo(VInfo); // @TODO: cache in scene renderer
		float FarPlane = std::max(FurthestLight, FurthestReflectionCapture) + 10.0f;
		//float FarPlane = VInfo.FarClipPlane;
		Data.LightGridZParams = GetLightGridZParams(VInfo.NearClipPlane, FarPlane);

		Data.RWNextCulledLightLinkIndex = RWNextCulledLightLinkBuffer->GetUavIndex();
		Data.RWCulledLightLinkIndex = RWCulledLightLinkBuffer->GetUavIndex();
		Data.RWStartGridOffsetIndex = RWStartGridOffsetBuffer->GetUavIndex();
		Data.RWNextCulledLightDataIndex = RWNextCulledLightDataBuffer->GetUavIndex();

		Data.CulledLightLinkIndex = RWCulledLightLinkBuffer->GetSrvIndex();
		Data.StartGridOffsetIndex = RWStartGridOffsetBuffer->GetSrvIndex();
		Data.RWLightGridNumOffsetIndex = RWLightGridNumOffsetBuffer->GetUavIndex();
		Data.RWLightGridLinkListIndex = RWLightGridLinkListBuffer->GetUavIndex();

		Data.LightGridNumOffsetIndex = RWLightGridNumOffsetBuffer->GetSrvIndex();
		Data.LightGridLinkListIndex = RWLightGridLinkListBuffer->GetSrvIndex();

		Data.NumReflectionCaptures = NumReflectionCaptures;
		Data.ReflectionCaptureBufferIndex = ReflectionCaptureBuffer->GetViewIndex();

		LightGridBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridData), EUniformBufferUsage::SingleFrame, &Data);

// ---------------------------------------------------------------------------------------------------------------------------------------

		PIXBeginEvent(CmdList->GetD3D12CommandList(), 1, "LightGrid");
		{
			CmdList->TransitionResourceWithTracking(RWNextCulledLightLinkBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->ClearUnorderedViewUInt(RWNextCulledLightLinkBuffer->GetUav(), 0);

			CmdList->TransitionResourceWithTracking(RWStartGridOffsetBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->ClearUnorderedViewUInt(RWStartGridOffsetBuffer->GetUav(), 0xFFFFFFFF);

			CmdList->TransitionResourceWithTracking(RWNextCulledLightDataBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->ClearUnorderedViewUInt(RWNextCulledLightDataBuffer->GetUav(), 0);
		}

		IntVector NumGroups = IntVector::DivideAndRoundUp(LightGridSize, LIGHT_GRID_INJECTION_GROUP_SIZE);
		{
			SCOPED_GPU_STAT(CmdList, "LightGridInjection");
			SCOPE_STAT();
			PIXBeginEvent( CmdList->GetD3D12CommandList(), 1, "Injection" );

			CmdList->TransitionResourceWithTracking(RWNextCulledLightLinkBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->TransitionResourceWithTracking(RWCulledLightLinkBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->TransitionResourceWithTracking(RWStartGridOffsetBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->FlushBarriers();

			CmdList->SetComputePipelineState(CommonResources::Get()->m_LightGridPSO->m_Injection_PSO);

			CmdList->SetComputeRootConstant(View->ViewBuffer->GetViewIndex(), 0);
			CmdList->SetComputeRootConstant(LightGridBuffer->GetViewIndex(), 1);

			CmdList->DispatchComputeShader(NumGroups.X, NumGroups.Y, NumGroups.Z);

			PIXEndEvent(CmdList->GetD3D12CommandList());
		}
		{
			SCOPED_GPU_STAT(CmdList, "LightGridCompact");
			SCOPE_STAT();
			PIXBeginEvent( CmdList->GetD3D12CommandList(), 1, "Compact" );

			CmdList->TransitionResourceWithTracking(RWStartGridOffsetBuffer->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			CmdList->TransitionResourceWithTracking(RWCulledLightLinkBuffer->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

			CmdList->TransitionResourceWithTracking(RWLightGridNumOffsetBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->TransitionResourceWithTracking(RWLightGridLinkListBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->TransitionResourceWithTracking(RWNextCulledLightDataBuffer->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CmdList->FlushBarriers();

			CmdList->SetComputePipelineState(CommonResources::Get()->m_LightGridPSO->m_Compact_PSO);

			CmdList->SetComputeRootConstant(View->ViewBuffer->GetViewIndex(), 0);
			CmdList->SetComputeRootConstant(LightGridBuffer->GetViewIndex(), 1);

			CmdList->DispatchComputeShader(NumGroups.X, NumGroups.Y, NumGroups.Z);

			PIXEndEvent(CmdList->GetD3D12CommandList());
		}

		PIXEndEvent(CmdList->GetD3D12CommandList());

#if WITH_EDITOR
		bool bDebug = true;
		if (bDebug)
		{
			World* OwningWorld = View->GetScene()->GetWorld();

			if (!DebugReadNumOffsetBuffer && OwningWorld->HasViewFlag(EWorldViewFlag::LightGrid))
			{
				{
					uint64 Size = RWLightGridNumOffsetBuffer->GetResource()->GetDesc().Width;
					DebugReadNumOffsetBuffer = new RenderRawBuffer(CmdList->GetParentDevice(), 0, Size, 0);

					RenderResource* NewResource = nullptr;
					CmdList->GetParentDevice()->CreateBuffer(D3D12_HEAP_TYPE_READBACK, Size, D3D12_RESOURCE_STATE_COMMON, false, &NewResource, "DebugReadNumOffsetBuffer", D3D12_RESOURCE_FLAG_NONE);

					DebugReadNumOffsetBuffer->m_ResourceLocation.AsStandAlone( NewResource );
					//OutTexture2D->SetMappedBaseAddress(NewResource->Map());


					CmdList->TransitionResourceWithTracking(RWLightGridNumOffsetBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
					CmdList->FlushBarriers();
					CmdList->GetD3D12CommandList()->CopyResource(DebugReadNumOffsetBuffer->GetResource()->GetResource(), RWLightGridNumOffsetBuffer->GetResource()->GetResource());
				}

				{
					uint64 Size = RWLightGridLinkListBuffer->GetResource()->GetDesc().Width;
					DebugReadListBuffer = new RenderRawBuffer(CmdList->GetParentDevice(), 0, Size, 0);

					RenderResource* NewResource = nullptr;
					CmdList->GetParentDevice()->CreateBuffer(D3D12_HEAP_TYPE_READBACK, Size, D3D12_RESOURCE_STATE_COMMON, false, &NewResource, "DebugReadListBuffer", D3D12_RESOURCE_FLAG_NONE);

					DebugReadListBuffer->m_ResourceLocation.AsStandAlone( NewResource );
					//OutTexture2D->SetMappedBaseAddress(NewResource->Map());


					CmdList->TransitionResourceWithTracking(RWLightGridLinkListBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE);
					CmdList->FlushBarriers();
					CmdList->GetD3D12CommandList()->CopyResource(DebugReadListBuffer->GetResource()->GetResource(), RWLightGridLinkListBuffer->GetResource()->GetResource());
				}

				DebugFenceValue = Renderer::Get()->GetFence()->Signal();
				DebugCachedData = Data;

				OwningWorld->SetViewFlag(EWorldViewFlag::LightGrid, false);
			}

			else if (DebugReadNumOffsetBuffer && Renderer::Get()->GetFence()->IsFenceComplete(DebugFenceValue))
			{
				std::vector<uint32> NumOffset;
				{
					uint64 Size = DebugReadNumOffsetBuffer->GetResource()->GetDesc().Width;
					NumOffset.resize(Size / 4);
					memcpy(NumOffset.data(), DebugReadNumOffsetBuffer->GetResource()->Map(), Size);
				}

				std::vector<uint32> LightList;
				{
					uint64 Size = DebugReadListBuffer->GetResource()->GetDesc().Width;
					LightList.resize(Size / LIGHT_GRID_INDEX_TYPE_SIZE);
					memcpy(LightList.data(), DebugReadListBuffer->GetResource()->Map(), Size);
				}

				DebugReadNumOffsetBuffer = nullptr;
				DebugReadListBuffer = nullptr;

				Matrix ProjetcionToWorld = View->GetSceneView().ProjectionToWorld;
				Matrix ViewToWorld = View->GetSceneView().ViewToWorld;

				for (int32 GridX = 0; GridX < DebugCachedData.CulledGridSize.GetX(); GridX++)
					for (int32 GridY = 0; GridY < DebugCachedData.CulledGridSize.GetY(); GridY++)
						for (int32 GridZ = 0; GridZ < DebugCachedData.CulledGridSize.GetZ(); GridZ++)
				{
					auto DrawLine = [](World* W, const Vector& Start, const Vector& End, Color InColor)
					{
						W->DrawDebugLine(Start, End, InColor, 0, 60);
					};

					int32 GridIndex = (GridZ * DebugCachedData.CulledGridSize.GetY() + GridY) * DebugCachedData.CulledGridSize.GetX() + GridX;
					int32 NumCulledLights = NumOffset[GridIndex * LIGHT_GRID_LIGHT_LINK_STRIDE + 0];
					int32 NumCulledReflections = NumOffset[(DebugCachedData.NumGridCells + GridIndex) * LIGHT_GRID_LIGHT_LINK_STRIDE + 0];

					//int32 NumCulledLights = LIGHT_GRID_DATA_TYPE_SIZE == 2
					//	? ((uint16*)NumOffset.data())[GridIndex * LIGHT_GRID_LIGHT_LINK_STRIDE + 0]
					//	: NumCulledLights = NumOffset[GridIndex * LIGHT_GRID_LIGHT_LINK_STRIDE + 0];

					if (NumCulledLights || NumCulledReflections)
					{
						Vector Vertices[2][2][2];
						for (uint32 Z = 0; Z < 2; Z++)
						{
							for (uint32 Y = 0; Y < 2; Y++)
							{
								for (uint32 X = 0; X < 2; X++)
								{
									float XStart	= (float)GridX / DebugCachedData.CulledGridSize.GetX();
									XStart = XStart * 2 - 1;
									float XEnd		= (float)(GridX + 1) / DebugCachedData.CulledGridSize.GetX();
									XEnd = XEnd * 2 - 1;

									float YStart	= (float)GridY / DebugCachedData.CulledGridSize.GetY();
									YStart = YStart * 2 - 1;
									float YEnd		= (float)(GridY + 1) / DebugCachedData.CulledGridSize.GetY();
									YEnd = YEnd * 2 - 1;

									float ZStart	= (float)GridZ / DebugCachedData.CulledGridSize.GetZ();
									float ZEnd		= (float)(GridZ + 1) / DebugCachedData.CulledGridSize.GetZ();

									float ProjectedX = X ? XStart : XEnd;
									float ProjectedY = Y ? YStart : YEnd;
									float ProjectedZ = Z ? ZStart : ZEnd;

									ProjectedY = -ProjectedY;


									float CellZ = (std::exp2((Z ? GridZ : GridZ + 1) / DebugCachedData.LightGridZParams.GetZ()) - DebugCachedData.LightGridZParams.GetY()) / DebugCachedData.LightGridZParams.GetX();
									if (GridZ == DebugCachedData.CulledGridSize.Z && Z == 0)
									{
										CellZ = 20000000;
										
									}

									ProjectedZ = View->GetSceneView().ConvertToDeviceZ(CellZ);

									Vector4 ProjectedVertex = Vector4(ProjectedX, ProjectedY, ProjectedZ, 1);
									Vector4 UnprojetcedVertex = ProjetcionToWorld.TransformVector4(ProjectedVertex);

									Vertices[X][Y][Z] = Vector(UnprojetcedVertex.GetX(), UnprojetcedVertex.GetY(), UnprojetcedVertex.GetZ()) / UnprojetcedVertex.GetW();
								}
							}
						}

						Color DrawColor = (NumCulledLights && NumCulledReflections) ? Color::White : (NumCulledLights ? Color::Green : Color::Blue);

						DrawLine(OwningWorld, Vertices[0][0][0], Vertices[0][0][1], DrawColor);
						DrawLine(OwningWorld, Vertices[1][0][0], Vertices[1][0][1], DrawColor);
						DrawLine(OwningWorld, Vertices[0][1][0], Vertices[0][1][1], DrawColor);
						DrawLine(OwningWorld, Vertices[1][1][0], Vertices[1][1][1], DrawColor);
						
						DrawLine(OwningWorld, Vertices[0][0][0], Vertices[0][1][0], DrawColor);
						DrawLine(OwningWorld, Vertices[1][0][0], Vertices[1][1][0], DrawColor);
						DrawLine(OwningWorld, Vertices[0][0][1], Vertices[0][1][1], DrawColor);
						DrawLine(OwningWorld, Vertices[1][0][1], Vertices[1][1][1], DrawColor);
						
						DrawLine(OwningWorld, Vertices[0][0][0], Vertices[1][0][0], DrawColor);
						DrawLine(OwningWorld, Vertices[0][1][0], Vertices[1][1][0], DrawColor);
						DrawLine(OwningWorld, Vertices[0][0][1], Vertices[1][0][1], DrawColor);
						DrawLine(OwningWorld, Vertices[0][1][1], Vertices[1][1][1], DrawColor);
					}
				}

			}
		}
#endif


	}

	void LightGrid::TransitionResourcesToRead( class D3D12CommandList* CmdList )
	{
		CmdList->TransitionResourceWithTracking(RWLightGridNumOffsetBuffer->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		CmdList->TransitionResourceWithTracking(RWLightGridLinkListBuffer->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

        }