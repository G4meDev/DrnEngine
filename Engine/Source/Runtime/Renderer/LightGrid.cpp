#include "DrnPCH.h"
#include "LightGrid.h"

#include "Runtime/Engine/DirectionalLightSceneProxy.h"
#include "Runtime/Engine/PointLightSceneProxy.h"
#include "Runtime/Engine/SpotLightSceneProxy.h"

namespace Drn
{
	Vector GetLightGridZParams(float NearPlane, float FarPlane)
	{
		double NearOffset = .095 * 100;
		double S = 4.05;

		double N = NearPlane + NearOffset;
		double F = FarPlane;

		double O = (F - N * exp2((LIGHT_GRID_SIZE_Z - 1) / S)) / (F - N);
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
		Data.HasDirectionalLight = 0;
		Data.NumCulledLights = 0;
		LocalLightData.clear();
		ViewSpacePosAndRadiusData.clear();
		ViewSpaceDirAndPreprocAngleData.clear();

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

				// @TODO: maybe use a tight bound sphere
				ViewSpacePosAndRadiusData.push_back(Vector4(View->GetSceneView().WorldToView.TransformPosition(SpotLight->GetWorldPosition()), SpotLight->GetAttenuation()));
				ViewSpaceDirAndPreprocAngleData.push_back((Vector4(View->GetSceneView().WorldToView.TransformVector(SpotLight->GetDirection()), GetTanRadAngleOrZero(SpotLight->GetOuterRadius()))));
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

		uint32 ActualNumLights = LocalLightData.size();
		drn_check(ActualNumLights < LIGHT_GRID_MAX_LOCAL_LIGHTS); // TODO: calculate tight fit for 65kb constant buffer target and clamp extra ones

		if (ActualNumLights == 0)
			LocalLightData.push_back({});
		if (ViewSpacePosAndRadiusData.empty())
			ViewSpacePosAndRadiusData.push_back({});
		if (ViewSpaceDirAndPreprocAngleData.empty())
			ViewSpaceDirAndPreprocAngleData.push_back({});

		LocalLightBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridLocalLightData) * LocalLightData.size(), EUniformBufferUsage::SingleFrame, LocalLightData.data());
		ViewSpacePosAndRadiusBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(Vector4) * ViewSpacePosAndRadiusData.size(), EUniformBufferUsage::SingleFrame, ViewSpacePosAndRadiusData.data());
		ViewSpaceDirAndPreprocAngleBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(Vector4) * ViewSpaceDirAndPreprocAngleData.size(), EUniformBufferUsage::SingleFrame, ViewSpaceDirAndPreprocAngleData.data());

		IntPoint ViewportSize = View->GetViewportSize();
		IntVector LightGridSize = IntVector(Math::DivideAndRoundUp(ViewportSize.X, LIGHT_GRID_PIXEL_SIZE), Math::DivideAndRoundUp(ViewportSize.Y, LIGHT_GRID_PIXEL_SIZE), LIGHT_GRID_SIZE_Z);

		Data.LocalLightBufferIndex = LocalLightBuffer->GetViewIndex();
		Data.ViewSpacePositionAndRadiusIndex = ViewSpacePosAndRadiusBuffer->GetViewIndex();
		Data.LightViewSpaceDirAndPreprocAngleIndex = ViewSpaceDirAndPreprocAngleBuffer->GetViewIndex();
		Data.NumCulledLights = ActualNumLights;
		Data.CulledGridSize = LightGridSize;
		Data.NumGridCells = LightGridSize.GetX() * LightGridSize.GetY() * LightGridSize.GetZ();
		Data.MaxCulledLightsPerCell = LIGHT_GRID_MAX_CULLED_LIGHT_PER_CELL;
		Data.LightGridPixelSizeShift = std::_Floor_of_log_2(LIGHT_GRID_PIXEL_SIZE);
		//Data.RWNumCulledLightsGridIndex = RWNumCulledLightsGridBuffer->;

		LightGridBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridData), EUniformBufferUsage::SingleFrame, &Data);

// ---------------------------------------------------------------------------------------------------------------------------------------


		D3D12CommandList* CmdList = View->GetCommandList();

		PIXBeginEvent(CmdList->GetD3D12CommandList(), 1, "LightGrid");
		{
			SCOPED_GPU_STAT(CmdList, "LightGridInjection");
			SCOPE_STAT();
			PIXBeginEvent( CmdList->GetD3D12CommandList(), 1, "Injection" );

			//m_CommandList->TransitionResourceWithTracking(m_HZBBuffer->M_HZBTarget->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			//m_CommandList->TransitionResourceWithTracking(m_GBuffer->m_DepthTarget->GetResource(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			CmdList->FlushBarriers();

			IntVector NumGroups = IntVector::DivideAndRoundUp(LightGridSize, LIGHT_GRID_INJECTION_GROUP_SIZE);

			CmdList->SetComputePipelineState(CommonResources::Get()->m_LightGridPSO->m_Injection_PSO);

			CmdList->SetComputeRootConstant(View->ViewBuffer->GetViewIndex(), 0);
			CmdList->SetComputeRootConstant(LightGridBuffer->GetViewIndex(), 1);

			CmdList->DispatchComputeShader(NumGroups.X, NumGroups.Y, NumGroups.Z);

			PIXEndEvent(CmdList->GetD3D12CommandList());
		}
		PIXEndEvent(CmdList->GetD3D12CommandList());

	}

}