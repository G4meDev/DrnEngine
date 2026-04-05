#pragma once

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

		void GenerateSkycubemap(class D3D12CommandList* CommandList, SceneRenderer* Renderer);

		void MapBuffer(class D3D12CommandList* CommandList, SceneRenderer* Renderer);

		TRefCountPtr<class RenderUniformBuffer> Buffer;
		ReflectionEnvironmentData m_Data;

		TRefCountPtr<RenderTextureCube> GeneratedCubemap;
		TRefCountPtr<RenderTextureCube> GeneratedCubemapIradiance;

		// TODO: better dirty condition
		std::string LastUsedCubemap;
	};
}