#pragma once

#include <memory>
#include <iosfwd>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <functional>
#include <chrono>

#include <DirectXMath.h>

#include "Runtime/Misc/StatsMisc.h"

#include "Runtime/Math/MathHelper.h"
#include "Runtime/Math/IntPoint.h"
#include "Runtime/Math/Vector.h"
#include "Runtime/Math/Vector4.h"
#include "Runtime/Math/Box.h"
#include "Runtime/Math/Sphere.h"
#include "Runtime/Math/Quat.h"
#include "Runtime/Math/Rotator.h"
#include "Runtime/Math/Transform.h"
#include "Runtime/Math/Matrix.h"
#include "Runtime/Math/Color.h"

class UpdateEventArgs;
class KeyEventArgs;
class ResizeEventArgs;
class WindowCloseEventArgs;

class Window;

struct aiMesh;
struct aiNode;
struct aiScene;

namespace Drn
{
	struct StaticMeshData;

	class PhysicManager;
	class PhysicScene;
	class BodyInstance;
	class BodySetup;

	class Device;
	class SwapChain;

	class World;
	class Level;
	class Scene;
	class SceneRenderer;
	class D3D12CommandList;
	class PrimitiveComponent;
	class PrimitiveSceneProxy;
	class LightSceneProxy;
	class StaticMeshComponent;
	class StaticMesh;
	class Material;
	class PipelineStateObject;
	class Resource;
	class Texture2D;
	class TextureVolume;
	class TextureCube;
	class CameraComponent;
	class Component;
	class SceneComponent;
	class Actor;
	class StaticMeshActor;
	class CameraActor;

	class AssetPreview;
	
	struct SystemFileNode;
}

class LogCategory;

#define LOG_DECLARE_CATEGORY(Category)					\
	extern LogCategory Category;						\

#define D3D12_DEBUG_LAYER 1
#define D3D12_GPU_VALIDATION 1
#define D3D12_Debug_INFO  1

#define NAME_NULL ""

#define DEFAULT_MATERIAL_PATH "\\Engine\\Content\\Materials\\M_DefaultMeshShader.drn"
#define DEFAULT_TEXTURE_PATH "\\Engine\\Content\\Textures\\Debug\\T_DebugIndexedColor.drn"
