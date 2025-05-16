#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogCommonResources);

namespace Drn
{
	class ScreenTriangle
	{
	public:

		ScreenTriangle( ID3D12GraphicsCommandList2* CommandList );
		~ScreenTriangle();

		Resource* m_VertexBuffer;
		Resource* m_IndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	};

	class ResolveAlphaBlendedPSO
	{
	public:

		ResolveAlphaBlendedPSO( ID3D12GraphicsCommandList2* CommandList );
		~ResolveAlphaBlendedPSO();
		
		ID3D12RootSignature* m_RootSignature;
		ID3D12PipelineState* m_PSO;
	};

	class CommonResources
	{
	public:

		CommonResources( ID3D12GraphicsCommandList2* CommandList );
		~CommonResources();

		static void Init( ID3D12GraphicsCommandList2* CommandList );
		static void Shutdown();

		inline static CommonResources* Get() { return m_SingletonInstance; }

		ScreenTriangle* m_ScreenTriangle;
		ResolveAlphaBlendedPSO* m_ResolveAlphaBlendedPSO;

	private:

		static CommonResources* m_SingletonInstance;
	};

	void CompileShaderString(const std::string& ShaderCode, const char* EntryPoint, const char* Profile, ID3DBlob*& ShaderBlob);
}