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

		class VertexBuffer* m_VertexBuffer;
		class IndexBuffer* m_IndexBuffer;

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList );
	};

	class BackfaceScreenTriangle
	{
	public:

		BackfaceScreenTriangle( ID3D12GraphicsCommandList2* CommandList );
		~BackfaceScreenTriangle();

		class VertexBuffer* m_VertexBuffer;
		class IndexBuffer* m_IndexBuffer;

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList );
	};

	class UniformQuad
	{
	public:

		UniformQuad( ID3D12GraphicsCommandList2* CommandList );
		~UniformQuad();

		class VertexBuffer* m_VertexBuffer;
		class IndexBuffer* m_IndexBuffer;

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList );
	};

	class PointLightSphere
	{
	public:

		PointLightSphere( ID3D12GraphicsCommandList2* CommandList );
		~PointLightSphere();

		class VertexBuffer* m_VertexBuffer;
		class IndexBuffer* m_IndexBuffer;

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList );
	};

	class SpotLightCone
	{
	public:

		SpotLightCone( ID3D12GraphicsCommandList2* CommandList );
		~SpotLightCone();

		class VertexBuffer* m_VertexBuffer;
		class IndexBuffer* m_IndexBuffer;

		void BindAndDraw( ID3D12GraphicsCommandList2* CommandList );
	};

	class ResolveAlphaBlendedPSO
	{
	public:

		ResolveAlphaBlendedPSO( ID3D12GraphicsCommandList2* CommandList );
		~ResolveAlphaBlendedPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class TonemapPSO
	{
	public:

		TonemapPSO( ID3D12GraphicsCommandList2* CommandList );
		~TonemapPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class AmbientOcclusionPSO
	{
	public:

		AmbientOcclusionPSO( ID3D12GraphicsCommandList2* CommandList );
		~AmbientOcclusionPSO();
		
		ID3D12PipelineState* m_SetupPSO;
		ID3D12PipelineState* m_HalfPSO;
		ID3D12PipelineState* m_MainPSO;
	};

	class ScreenSpaceReflectionPSO
	{
	public:

		ScreenSpaceReflectionPSO( ID3D12GraphicsCommandList2* CommandList );
		~ScreenSpaceReflectionPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class ResolveEditorSelectionPSO
	{
	public:

		ResolveEditorSelectionPSO( ID3D12GraphicsCommandList2* CommandList );
		~ResolveEditorSelectionPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class SpriteEditorPrimitivePSO
	{
	public:

		SpriteEditorPrimitivePSO( ID3D12GraphicsCommandList2* CommandList );
		~SpriteEditorPrimitivePSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class SpriteHitProxyPSO
	{
	public:

		SpriteHitProxyPSO( ID3D12GraphicsCommandList2* CommandList );
		~SpriteHitProxyPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class LightPassPSO
	{
	public:

		LightPassPSO( ID3D12GraphicsCommandList2* CommandList );
		~LightPassPSO();

		ID3D12PipelineState* m_PSO;
	};

	class DebugLineThicknessPSO
	{
	public:

		DebugLineThicknessPSO( ID3D12GraphicsCommandList2* CommandList );
		~DebugLineThicknessPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class DebugLinePSO
	{
	public:

		DebugLinePSO( ID3D12GraphicsCommandList2* CommandList );
		~DebugLinePSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class HZBPSO
	{
	public:

		HZBPSO( ID3D12GraphicsCommandList2* CommandList );
		~HZBPSO();
		
		ID3D12PipelineState* m_1Mip_PSO;
		ID3D12PipelineState* m_2Mip_PSO;
		ID3D12PipelineState* m_3Mip_PSO;
		ID3D12PipelineState* m_4Mip_PSO;
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
		BackfaceScreenTriangle* m_BackfaceScreenTriangle;
		UniformQuad* m_UniformQuad;
		PointLightSphere* m_PointLightSphere;
		SpotLightCone* m_SpotLightCone;

		ResolveAlphaBlendedPSO* m_ResolveAlphaBlendedPSO;
		ResolveEditorSelectionPSO* m_ResolveEditorSelectionPSO;
		TonemapPSO* m_TonemapPSO;
		AmbientOcclusionPSO* m_AmbientOcclusionPSO;
		LightPassPSO* m_LightPassPSO;
		ScreenSpaceReflectionPSO* m_ScreenSpaceReflectionPSO;

		SpriteEditorPrimitivePSO* m_SpriteEditorPrimitivePSO;
		SpriteHitProxyPSO* m_SpriteHitProxyPSO;

		DebugLineThicknessPSO* m_DebugLineThicknessPSO;
		DebugLinePSO* m_DebugLinePSO;

		HZBPSO* m_HZBPSO;

		AssetHandle<Texture2D> m_SSAO_Random;

	private:

		static CommonResources* m_SingletonInstance;
	};

	void CompileShaderString(const std::wstring& ShaderPath, const char* EntryPoint, const char* Profile, ID3DBlob*& ShaderBlob, const D3D_SHADER_MACRO* Macros = NULL);
	bool CompileShader( const std::wstring& ShaderPath, const wchar_t* EntryPoint, const wchar_t* Profile, const std::vector<const wchar_t*>& Macros, ID3DBlob** ByteBlob );
}