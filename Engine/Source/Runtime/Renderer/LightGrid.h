#pragma once

#include "ForwardTypes.h"

#define LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE	5
#define LIGHT_GRID_MAX_LOCAL_LIGHTS			D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT / LIGHT_GRID_LOCAL_LIGHT_DATA_STRIDE

#define LIGHT_GRID_LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_GRID_LIGHT_TYPE_POINT			1
#define LIGHT_GRID_LIGHT_TYPE_SPOT			2
#define LIGHT_GRID_LIGHT_TYPE_MAX			3

#define LIGHT_GRID_MAX_CULLED_LIGHT_PER_CELL 32

#define LIGHT_GRID_STRIDE_CULLED_LIGHT			2
#define LIGHT_GRID_NUM_CULLED_PRIMITIVE_TYPES	2
#define LIGHT_GRID_LIGHT_LINK_STRIDE			2

#define LIGHT_GRID_PIXEL_SIZE 64
#define LIGHT_GRID_SIZE_Z 32
#define LIGHT_GRID_INJECTION_GROUP_SIZE 4

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
		uint32 LocalLightBufferIndex;

		IntVector CulledGridSize;
		uint32 NumCulledLights;

		uint32 NumGridCells;
		uint32 MaxCulledLightsPerCell;
		uint32 LightGridPixelSizeShift;
		uint32 ViewSpacePositionAndRadiusIndex;

		Vector LightGridZParams;
		uint32 LightViewSpaceDirAndPreprocAngleIndex;

		uint32 RWNumCulledLightsGridIndex;
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

		std::vector<Vector4> ViewSpacePosAndRadiusData;
		TRefCountPtr<class RenderUniformBuffer> ViewSpacePosAndRadiusBuffer;

		std::vector<Vector4> ViewSpaceDirAndPreprocAngleData;
		TRefCountPtr<class RenderUniformBuffer> ViewSpaceDirAndPreprocAngleBuffer;

		TRefCountPtr<class RenderUniformBuffer> RWNumCulledLightsGridBuffer;
	};
}