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

		uint32 LightIndex = 0;
		for (BitArray::ConstSetBitIterator It(View->GetVisibleLights()); It; ++It)
		{
			LightSceneProxy* Proxy = View->GetScene()->GetLightProxies()[It.GetIndex()];

			if (Proxy->GetLightType() == ELightType::PointLight)
			{
				PointLightSceneProxy* PointLight = static_cast<PointLightSceneProxy*>(Proxy);
				uint32 LightType = LIGHT_GRID_LIGHT_TYPE_POINT;

				Data.LocalLightData[LightIndex].LightPositionAndInvRadius = Vector4(PointLight->GetWorldPosition(), 1.0f / PointLight->GetRadius());
				Data.LocalLightData[LightIndex].LightColorAndFalloffExponent = Vector4(PointLight->GetColor(), 0.0f);
				Data.LocalLightData[LightIndex].LightDirectionAndLightType = Vector4(Vector::ZeroVector, *((float*)&LightType));
				Data.LocalLightData[LightIndex].SpotAnglesAndSourceRadiusPacked = Vector4(0.0f, 0.0f, PointLight->GetRadius(), 0.0f);

				LightIndex++;
			}

			else if (Proxy->GetLightType() == ELightType::SpotLight)
			{
				SpotLightSceneProxy* SpotLight = static_cast<SpotLightSceneProxy*>(Proxy);
				uint32 LightType = LIGHT_GRID_LIGHT_TYPE_SPOT;

				Data.LocalLightData[LightIndex].LightPositionAndInvRadius = Vector4(SpotLight->GetWorldPosition(), 1.0f / SpotLight->GetAttenuation());
				Data.LocalLightData[LightIndex].LightColorAndFalloffExponent = Vector4(SpotLight->GetColor(), 0.0f);
				Data.LocalLightData[LightIndex].LightDirectionAndLightType = Vector4(SpotLight->GetDirection(), *((float*)&LightType));
				Data.LocalLightData[LightIndex].SpotAnglesAndSourceRadiusPacked = Vector4(SpotLight->GetCosOuterCone(), SpotLight->GetInvCosConeDifference(), SpotLight->GetAttenuation(), 0.0f);

				LightIndex++;
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

		Data.NumCulledLights = LightIndex;
		LightGridBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridData), EUniformBufferUsage::SingleFrame, &Data);
	}

}