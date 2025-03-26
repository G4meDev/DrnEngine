#include <memory>
#include <iosfwd>
#include <string>

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
}

namespace Drn
{
	class Archive;
	class AssetPreview;
	
	struct SystemFileNode;

}

class LogCategory;

#define LOG_DECLARE_CATEGORY(Category)					\
	extern LogCategory Category;						\

#define NAME_NULL ""