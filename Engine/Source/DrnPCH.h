#pragma once

#include "Runtime/Core/Drn.h"
#include "Runtime/Core/AssetManager.h"
#include "Runtime/Core/Archive.h"
#include "Runtime/Core/Time.h"

#include "Runtime/Engine/WorldManager.h"
#include "Runtime/Engine/World.h"
#include "Runtime/Engine/Level.h"
#include "Runtime/Engine/Scene.h"
#include "Runtime/Engine/PrimitiveComponent.h"
#include "Runtime/Engine/PrimitiveSceneProxy.h"
#include "Runtime/Engine/StaticMesh.h"
#include "Runtime/Engine/StaticMeshComponent.h"
#include "Runtime/Engine/CameraComponent.h"
#include "Runtime/Engine/Component.h"
#include "Runtime/Engine/SceneComponent.h"
#include "Runtime/Engine/Actor.h"
#include "Runtime/Engine/StaticMeshActor.h"
#include "Runtime/Engine/CameraActor.h"

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

#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/SceneRenderer.h"
#include "Runtime/Renderer/VertexLayout.h"
#include "Runtime/Renderer/D3D12Utils.h"
#include "Runtime/Renderer/Material/Material.h"

#include "Runtime/Math/Math.h"
#include "Runtime/Math/Box.h"
#include "Runtime/Math/IntPoint.h"

#include "Runtime/Physic/PhysicCore.h"
#include "Runtime/Physic/PhysicManager.h"
#include "Runtime/Physic/PhysicScene.h"
#include "Runtime/Physic/BodyInstance.h"
#include "Runtime/Physic/BodySetup.h"
#include "Runtime/Physic/AggregateGeom.h"
#include "Runtime/Physic/PhysicUserData.h"


#include "Runtime/Misc/StringHelper.h"
#include "Runtime/Misc/Log.h"
#include "Runtime/Misc/DateTime.h"
#include "Runtime/Misc/EnumClassFlags.h"
#include "Runtime/Misc/Path.h"
#include "Runtime/Misc/FileSystem.h"
#include "Runtime/Misc/Profiler.h"

#include <iostream>
#include <WinUser.h>

#include <sstream>
#include <ostream>
#include <fstream>
#include <stdlib.h>
#include <regex>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include "Runtime/Renderer/d3dx12.h"
#include <dxgidebug.h>
#include <wrl.h>

#include <chrono>

#include <algorithm>
#include <filesystem>
//#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>