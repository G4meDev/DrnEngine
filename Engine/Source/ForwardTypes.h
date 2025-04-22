#include <memory>
#include <iosfwd>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>

#include <DirectXMath.h>

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

namespace dx12lib
{
	class Device;
	class CommandList;
	class SwapChain;
	class VertexBuffer;
	class IndexBuffer;
	class RootSignature;
	class PipelineStateObject;
	class Texture;
	class RenderTarget;
}

namespace std
{
	//class ofstream;
	//class set;
}

namespace Drn
{
	struct StaticMeshVertexData;
	struct StaticMeshVertexBuffer;
	struct StaticMeshData;
	struct StaticMeshRenderProxy;
	struct MaterialData;

	class PhysicManager;
	class PhysicScene;
	class BodyInstance;
	class BodySetup;

	struct VertexPosColor;

	class World;
	class Level;
	class Scene;
	class SceneRenderer;
	class PrimitiveComponent;
	class PrimitiveSceneProxy;
	class StaticMeshComponent;
	class StaticMesh;
	class CameraComponent;
	class Component;
	class SceneComponent;
	class Actor;
	class StaticMeshActor;
	class CameraActor;
	
	class Archive;
	class AssetPreview;
	
	struct SystemFileNode;

	struct IntPoint;
}

class LogCategory;

#define LOG_DECLARE_CATEGORY(Category)					\
	extern LogCategory Category;						\

#define NAME_NULL ""