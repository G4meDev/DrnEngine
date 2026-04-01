#pragma once

#include "ForwardTypes.h"

#define LIGHT_GRID_MAX_LOCAL_LIGHTS			100
#define LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE	5

namespace Drn
{
	class SceneRenderer;

	struct LightGridData
	{
		LightGridData() {}
		~LightGridData() {}

		Vector DirectionalLightColor;
		uint32 HasDirectionalLight;
		Vector DirectionalLightDirection;
		uint32 NumCulledLights;

		union
		{
			struct LightGridDirectionalLightData
			{
				Vector4 LightPositionAndInvRadius;
				Vector4 LightColorAndFalloffExponent;
				Vector4 SpotAnglesAndSourceRadiusPacked;
				Vector4 LightDirectionAndShadowMask;
				Vector4 LightTangentAndSoftSourceRadius;
			} LocalLightData[LIGHT_GRID_MAX_LOCAL_LIGHTS];

			Vector4 LocalLightPackedData[LIGHT_GRID_MAX_LOCAL_LIGHTS * LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE];
		};

	};

	class LightGrid
	{
	public:
		LightGrid(SceneRenderer* InView) : View(InView) {};
		~LightGrid() {};

		void ComputeLightGrid();

	private:
		class SceneRenderer* View;
		TRefCountPtr<class RenderUniformBuffer> LightGridBuffer;
		LightGridData Data;
	};
}