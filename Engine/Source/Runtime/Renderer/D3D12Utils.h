#pragma once

#include "Windows.h"

#define NUM_BACKBUFFERS 3

#define DISPLAY_OUTPUT_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define DEPTH_FORMAT DXGI_FORMAT_D32_FLOAT
#define DEPTH_STENCIL_FORMAT DXGI_FORMAT_D24_UNORM_S8_UINT

#define GBUFFER_GUID_FORMAT DXGI_FORMAT_R32G32B32A32_UINT

#define GBUFFER_COLOR_DEFERRED_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT
#define GBUFFER_BASE_COLOR_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define GBUFFER_MASKS_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define GBUFFER_WORLD_NORMAL_FORMAT DXGI_FORMAT_R10G10B10A2_UNORM

#define VERIFYD3D12RESULT(x)			{HRESULT hres = x; if (FAILED(hres)) { VerifyD3D12Result(hres, #x, __FILE__, __LINE__); }}

//void VerifyD3D12Result(HRESULT Result, const char* Code, const char* Filename, UINT Line);