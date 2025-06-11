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
#include "Runtime/Engine/PointLightActor.h"

#include "Runtime/Components/BillboardComponent.h"

#include "Runtime/Renderer/Device.h"
#include "Runtime/Renderer/SwapChain.h"

#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/CommonResources.h"
#include "Runtime/Renderer/SceneRenderer.h"
#include "Runtime/Renderer/PipelineStateObject.h"
#include "Runtime/Renderer/Resource.h"
#include "Runtime/Renderer/VertexBuffer.h"
#include "Runtime/Renderer/IndexBuffer.h"
#include "Runtime/Renderer/InputLayout.h"
#include "Runtime/Renderer/D3D12Utils.h"
#include "Runtime/Renderer/Material/Material.h"
#include "Runtime/Renderer/Texture/Texture2D.h"
#include "Runtime/Renderer/Texture/TextureVolume.h"
#include "Runtime/Renderer/Texture/TextureCube.h"

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

//#define PAR_SHAPES_IMPLEMENTATION
//#include "ThirdParty/par/par_shapes.h"

#include <iostream>
#include <WinUser.h>

#include <sstream>
#include <ostream>
#include <fstream>
#include <stdlib.h>
#include <regex>
#include <format>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include "Runtime/Renderer/d3dx12.h"
#include <dxgidebug.h>
#include <wrl.h>
#include <pix3.h>

#include <chrono>

#include <algorithm>
#include <filesystem>
//#include <functional>

#include <taskflow.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

