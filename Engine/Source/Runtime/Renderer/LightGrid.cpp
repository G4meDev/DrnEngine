#include "DrnPCH.h"
#include "LightGrid.h"

#include "Runtime/Engine/DirectionalLightSceneProxy.h"

namespace Drn
{
	void LightGrid::ComputeLightGrid()
	{
		Data.HasDirectionalLight = 0;
		Data.NumCulledLights = 0;

		for (BitArray::ConstSetBitIterator It(View->GetVisibleLights()); It; ++It)
		{
			LightSceneProxy* Proxy = View->GetScene()->GetLightProxies()[It.GetIndex()];

			if (Proxy->GetLightType() == ELightType::DirectionalLight)
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

		LightGridBuffer = RenderUniformBuffer::Create(View->GetCommandList()->GetParentDevice(), sizeof(LightGridData), EUniformBufferUsage::SingleFrame, &Data);
	}

}