#pragma once

#include "Runtime/Core/Drn.h"

#include <dx12lib/Device.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/VertexBuffer.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Texture.h>
#include <dx12lib/Helpers.h>

#include "Runtime/Renderer/D3D12Utils.h"
#include "Runtime/Math/Math.h"
#include "Runtime/Math/IntPoint.h"
#include "Runtime/Containers/List.h"

#include "Runtime/Misc/Log.h"
#include "Runtime/Misc/DateTime.h"
#include "Runtime/Misc/EnumClassFlags.h"
#include "Runtime/Misc/Path.h"

#include <iostream>
#include <WinUser.h>

//#include <memory>
#include <sstream>
#include <ostream>
#include <fstream>
#include <stdlib.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include "Runtime/Renderer/d3dx12.h"
#include <dxgidebug.h>
#include <wrl.h>

#include <chrono>

#include <string>
#include <unordered_map>

#include <algorithm>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>