#pragma once

#include <memory>
#include <iosfwd>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <chrono>

#include <DirectXMath.h>

#include "Runtime/Misc/StatsMisc.h"

#include "Runtime/Math/IntPoint.h"
#include "Runtime/Math/Vector.h"
#include "Runtime/Math/Box.h"
#include "Runtime/Math/Quat.h"
#include "Runtime/Math/Rotator.h"
#include "Runtime/Math/Transform.h"
#include "Runtime/Math/Matrix.h"

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
	class PrimitiveComponent;
	class PrimitiveSceneProxy;
	class StaticMeshComponent;
	class StaticMesh;
	class Material;
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

#define NAME_NULL ""

#define DEFAULT_MATERIAL_PATH "\\Engine\\Content\\Materials\\M_DefaultMeshShader.drn"