#include <memory>

class UpdateEventArgs;
class KeyEventArgs;
class ResizeEventArgs;
class WindowCloseEventArgs;

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

class LogCategory;

#define LOG_DECLARE_CATEGORY(Category)					\
	extern LogCategory Category;						\