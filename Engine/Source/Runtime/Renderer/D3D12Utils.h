#pragma once

#include "Windows.h"
#include <d3d12.h>
#include "Runtime/Renderer/RenderResource.h"

#define NUM_BACKBUFFERS 3
#define NUM_SCENE_DOWNSAMPLES 6
#define NUM_BLOOM_TARGETS NUM_SCENE_DOWNSAMPLES * 2

#define BLOOM_STATIC_SAMPLE_COUNT 7
#define BLOOM_PACKED_SAMPLE_COUNT ( BLOOM_STATIC_SAMPLE_COUNT + 1 ) / 2

#define DISPLAY_OUTPUT_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define DEPTH_FORMAT DXGI_FORMAT_D32_FLOAT
#define DEPTH_STENCIL_FORMAT DXGI_FORMAT_D24_UNORM_S8_UINT

#define GBUFFER_GUID_FORMAT DXGI_FORMAT_R32G32B32A32_UINT

#define GBUFFER_COLOR_DEFERRED_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT
#define GBUFFER_BASE_COLOR_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
#define GBUFFER_MASKS_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define GBUFFER_VELOCITY_FORMAT DXGI_FORMAT_R16G16_UNORM
#define GBUFFER_WORLD_NORMAL_FORMAT DXGI_FORMAT_R16G16_FLOAT

#define DECAL_BASE_COLOR_FORMAT GBUFFER_BASE_COLOR_FORMAT
#define DECAL_MASKS_FORMAT GBUFFER_MASKS_FORMAT
#define DECAL_NORMAL_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM

#define SCENE_DOWN_SAMPLE_FORMAT DXGI_FORMAT_R11G11B10_FLOAT
#define BLOOM_FORMAT SCENE_DOWN_SAMPLE_FORMAT

#define EDITOR_PRIMITIVE_COLOR_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM_SRGB


#define D3D12_RESOURCE_STATE_TBD D3D12_RESOURCE_STATES(-1 ^ (1 << 31))
#define D3D12_RESOURCE_STATE_CORRUPT D3D12_RESOURCE_STATES(-2 ^ (1 << 31))

static bool IsValidD3D12ResourceState(D3D12_RESOURCE_STATES InState)
{
	return (InState != D3D12_RESOURCE_STATE_TBD && InState != D3D12_RESOURCE_STATE_CORRUPT);
}

#define MAX_TEXTURE_SIZE_2D 4096
#define MAX_TEXTURE_SIZE_CUBE 2048

#define VERIFYD3D12RESULT(x)			{HRESULT hres = x; if (FAILED(hres)) { VerifyD3D12Result(hres, #x, __FILE__, __LINE__); }}

void SetName(class ID3D12Object* const Object, const std::string& Name);
void SetName(class Drn::RenderResource* const Resource, const std::string& Name);

inline bool IsCPUWritable(D3D12_HEAP_TYPE HeapType, const D3D12_HEAP_PROPERTIES *pCustomHeapProperties = nullptr)
{
	return HeapType == D3D12_HEAP_TYPE_UPLOAD;
}

inline bool IsCPUInaccessible(D3D12_HEAP_TYPE HeapType, const D3D12_HEAP_PROPERTIES *pCustomHeapProperties = nullptr)
{
	return HeapType == D3D12_HEAP_TYPE_DEFAULT;
}

inline D3D12_RESOURCE_STATES DetermineInitialResourceState(D3D12_HEAP_TYPE HeapType, const D3D12_HEAP_PROPERTIES *pCustomHeapProperties = nullptr)
{
	if (HeapType == D3D12_HEAP_TYPE_DEFAULT || IsCPUWritable(HeapType, pCustomHeapProperties))
	{
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else
	{
		return D3D12_RESOURCE_STATE_COPY_DEST;
	}
}

inline DXGI_FORMAT FindDepthStencilParentDXGIFormat(DXGI_FORMAT InFormat)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_R24G8_TYPELESS;
		// Changing Depth Buffers to 32 bit on Dingo as D24S8 is actually implemented as a 32 bit buffer in the hardware
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_R32G8X24_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_TYPELESS;
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_TYPELESS;
	};
	return InFormat;
}

static uint8 GetPlaneSliceFromViewFormat(DXGI_FORMAT ResourceFormat, DXGI_FORMAT ViewFormat)
{
	// Currently, the only planar resources used are depth-stencil formats
	switch (FindDepthStencilParentDXGIFormat(ResourceFormat))
	{
	case DXGI_FORMAT_R24G8_TYPELESS:
		switch (ViewFormat)
		{
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			return 0;
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			return 1;
		}
		break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		switch (ViewFormat)
		{
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			return 0;
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			return 1;
		}
		break;
	case DXGI_FORMAT_NV12:
		switch (ViewFormat)
		{
		case DXGI_FORMAT_R8_UNORM:
			return 0;
		case DXGI_FORMAT_R8G8_UNORM:
			return 1;
		}
		break;
	}

	return 0;
}

inline bool HasStencilBits(DXGI_FORMAT InFormat)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return true;
	};

	return false;
}

inline DXGI_FORMAT FindShaderResourceDXGIFormat(DXGI_FORMAT InFormat, bool bSRGB)
{
	if (bSRGB)
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case DXGI_FORMAT_BC1_TYPELESS:         return DXGI_FORMAT_BC1_UNORM_SRGB;
		case DXGI_FORMAT_BC2_TYPELESS:         return DXGI_FORMAT_BC2_UNORM_SRGB;
		case DXGI_FORMAT_BC3_TYPELESS:         return DXGI_FORMAT_BC3_UNORM_SRGB;
		case DXGI_FORMAT_BC7_TYPELESS:         return DXGI_FORMAT_BC7_UNORM_SRGB;
		};
	}
	else
	{
		switch (InFormat)
		{
		case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_BC1_TYPELESS:      return DXGI_FORMAT_BC1_UNORM;
		case DXGI_FORMAT_BC2_TYPELESS:      return DXGI_FORMAT_BC2_UNORM;
		case DXGI_FORMAT_BC3_TYPELESS:      return DXGI_FORMAT_BC3_UNORM;
		case DXGI_FORMAT_BC7_TYPELESS:      return DXGI_FORMAT_BC7_UNORM;
		};
	}
	switch (InFormat)
	{
	case DXGI_FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_R16_TYPELESS: return DXGI_FORMAT_R16_UNORM;
	case DXGI_FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}
	return InFormat;
}

inline DXGI_FORMAT FindUnorderedAccessDXGIFormat(DXGI_FORMAT InFormat)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	return InFormat;
}

inline DXGI_FORMAT FindDepthStencilDXGIFormat(DXGI_FORMAT InFormat)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_R24G8_TYPELESS:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case DXGI_FORMAT_R32G8X24_TYPELESS:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case DXGI_FORMAT_R32_TYPELESS:
		return DXGI_FORMAT_D32_FLOAT;
	case DXGI_FORMAT_R16_TYPELESS:
		return DXGI_FORMAT_D16_UNORM;
	};
	return InFormat;
}

inline DXGI_FORMAT FindDepthStencilSRVFormat(DXGI_FORMAT InFormat, bool StencilView = false)
{
	switch (InFormat)
	{
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return StencilView ? DXGI_FORMAT_X24_TYPELESS_G8_UINT : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	//case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	//	return StencilView ? DXGI_FORMAT_X32_TYPELESS_G8_UINT : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	};
	return InFormat;
}