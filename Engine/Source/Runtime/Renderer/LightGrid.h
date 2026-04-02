#pragma once

#include "ForwardTypes.h"

#define LIGHT_GRID_MAX_LOCAL_LIGHTS			100
#define LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE	5

#define LIGHT_GRID_LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_GRID_LIGHT_TYPE_POINT			1
#define LIGHT_GRID_LIGHT_TYPE_SPOT			2
#define LIGHT_GRID_LIGHT_TYPE_MAX			3

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
		uint32 LocalLightBufferIndex;
	};

	// update data stride if changed
	struct LightGridLocalLightData
	{
		Vector4 LightPositionAndInvRadius;
		Vector4 LightColorAndFalloffExponent;
		Vector4 LightDirectionAndLightType;
		Vector4 SpotAnglesAndSourceRadiusPacked;
		Vector4 LightTangentAndSoftSourceRadius;
	};
	
	class LightGrid
	{

	public:
		LightGrid(SceneRenderer* InView) : View(InView) {}
		~LightGrid() {}

		void ComputeLightGrid();

		inline class RenderUniformBuffer* GetBuffer() const { return LightGridBuffer; }

	private:
		class SceneRenderer* View;
		TRefCountPtr<class RenderUniformBuffer> LightGridBuffer;
		LightGridData Data;
		std::vector<LightGridLocalLightData> LocalLightData;
		TRefCountPtr<class RenderUniformBuffer> LocalLightBuffer;
	};
}