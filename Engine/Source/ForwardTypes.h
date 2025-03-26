#include <memory>
#include <iosfwd>
#include <string>
#include <vector>
#include <set>

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
	class World;
	class Scene;
	class SceneRenderer;
	class PrimitiveComponent;
	
	class Archive;
	class AssetPreview;
	
	struct SystemFileNode;

}

class LogCategory;

#define LOG_DECLARE_CATEGORY(Category)					\
	extern LogCategory Category;						\

#define NAME_NULL ""