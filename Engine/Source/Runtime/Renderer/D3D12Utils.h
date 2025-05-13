#pragma once

#include "Windows.h"

#define NUM_BACKBUFFERS 3

#define D3D12_DEBUG_LAYER 1

#define DISPLAY_OUTPUT_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define DEPTH_FORMAT DXGI_FORMAT_D32_FLOAT

#define GBUFFER_GUID_FORMAT DXGI_FORMAT_R32G32B32A32_UINT


#define VERIFYD3D12RESULT(x)			{HRESULT hres = x; if (FAILED(hres)) { VerifyD3D12Result(hres, #x, __FILE__, __LINE__); }}

void VerifyD3D12Result(HRESULT Result, const char* Code, const char* Filename, UINT Line);