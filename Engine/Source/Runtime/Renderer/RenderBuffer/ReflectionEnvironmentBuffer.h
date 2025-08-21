#include "ForwardTypes.h"
#include "Runtime/Renderer/RenderBuffer/RenderBuffer.h"

namespace Drn
{
	struct ReflectionEnvironmentData
	{
	public:
		ReflectionEnvironmentData() = default;

		uint32 BaseColorTexture;
		uint32 WorldNormalTexture;
		uint32 MasksTexture;
		uint32 DepthTexture;

		uint32 SSRTexture;
		uint32 AOTexture;
		uint32 PreintegratedGFTexture;
		uint32 SkyCubemapTexture;

		Vector SkyLightColor;
		uint32 SkyLightMipCount;
	};

	class ReflectionEnvironmentBuffer : public RenderBuffer
	{
	public:

		ReflectionEnvironmentBuffer();
		virtual ~ReflectionEnvironmentBuffer();

		virtual void Init() override;
		virtual void Resize( const IntPoint& Size ) override;

		virtual void Clear( ID3D12GraphicsCommandList2* CommandList ) override;
		virtual void Bind( ID3D12GraphicsCommandList2* CommandList ) override;

		void MapBuffer( ID3D12GraphicsCommandList2* CommandList, SceneRenderer* Renderer);
		void ReleaseBuffers();

		Resource* m_Buffer;
		ReflectionEnvironmentData m_Data;
	};
}