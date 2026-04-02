#include "DrnPCH.h"
#include "LightGrid.h"

#include "Runtime/Engine/DirectionalLightSceneProxy.h"
#include "Runtime/Engine/PointLightSceneProxy.h"
#include "Runtime/Engine/SpotLightSceneProxy.h"

namespace Drn
{
	void LightGrid::ComputeLightGrid()
	{
		Data.HasDirectionalLight = 0;
		Data.NumCulledLights = 0;
		LocalLightData.clear();

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
		{
			LocalLightData.push_back({});
		}
		LocalLightBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridLocalLightData) * LocalLightData.size(), EUniformBufferUsage::SingleFrame, LocalLightData.data());

		Data.NumCulledLights = ActualNumLights;
		Data.LocalLightBufferIndex = LocalLightBuffer->GetViewIndex();
		LightGridBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridData), EUniformBufferUsage::SingleFrame, &Data);
	}

}