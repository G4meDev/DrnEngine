#pragma once

#include "ForwardTypes.h"

LOG_DECLARE_CATEGORY(LogCommonResources);

namespace Drn
{
	//struct ID3D12GraphicsCommandList2;
	class D3D12CommandList;
	class GraphicsPipelineState;
	class CommonResources;
	class RenderVertexBuffer;
	class RenderIndexBuffer;

	class ScreenTriangle
	{
	public:

		ScreenTriangle( D3D12CommandList* CommandList );
		~ScreenTriangle();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );
	};

	class BackfaceScreenTriangle
	{
	public:

		BackfaceScreenTriangle( D3D12CommandList* CommandList );
		~BackfaceScreenTriangle();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );
	};

	class UniformQuad
	{
	public:

		UniformQuad( D3D12CommandList* CommandList );
		~UniformQuad();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );

		uint32 VertexCount;
		uint32 PrimitiveCount;
	};

	class UniformCube
	{
	public:

		UniformCube( D3D12CommandList* CommandList );
		~UniformCube();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );

		uint32 VertexCount;
		uint32 PrimitiveCount;
	};

	class UniformCubePositionOnly
	{
	public:

		UniformCubePositionOnly( D3D12CommandList* CommandList );
		~UniformCubePositionOnly();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );

		uint32 VertexCount;
		uint32 PrimitiveCount;
	};

	class PointLightSphere
	{
	public:

		PointLightSphere( D3D12CommandList* CommandList );
		~PointLightSphere();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );

		uint32 VertexCount;
		uint32 PrimitiveCount;
	};

	class SpotLightCone
	{
	public:

		SpotLightCone( D3D12CommandList* CommandList );
		~SpotLightCone();

		TRefCountPtr<RenderVertexBuffer> m_VertexBuffer;
		TRefCountPtr<RenderIndexBuffer> m_IndexBuffer;

		void BindAndDraw( D3D12CommandList* CommandList );

		uint32 VertexCount;
		uint32 PrimitiveCount;
	};

	class ResolveAlphaBlendedPSO : public RefCountedObject
	{
	public:

		ResolveAlphaBlendedPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class TonemapPSO : public RefCountedObject
	{
	public:
		TonemapPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class AmbientOcclusionPSO : public RefCountedObject
	{
	public:
		AmbientOcclusionPSO( D3D12CommandList* CommandList, CommonResources* CR );
		
		TRefCountPtr<GraphicsPipelineState> m_SetupPSO;
		TRefCountPtr<GraphicsPipelineState> m_HalfPSO;
		TRefCountPtr<GraphicsPipelineState> m_MainPSO;
	};

	class ScreenSpaceReflectionPSO : public RefCountedObject
	{
	public:
		ScreenSpaceReflectionPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class ReflectionEnvironemntPSO : public RefCountedObject
	{
	public:
		ReflectionEnvironemntPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class TAAPSO
	{
	public:

		TAAPSO( ID3D12GraphicsCommandList2* CommandList );
		~TAAPSO();
		
		ID3D12PipelineState* m_PSO;
	};

	class SceneDownSamplePSO : public RefCountedObject
	{
	public:
		SceneDownSamplePSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class BloomPSO : public RefCountedObject
	{
	public:
		BloomPSO( D3D12CommandList* CommandList, CommonResources* CR );
		
		TRefCountPtr<GraphicsPipelineState> m_BloomYPSO;
		TRefCountPtr<GraphicsPipelineState> m_BloomXPSO;
		TRefCountPtr<GraphicsPipelineState> m_BloomXAddtivePSO;
	};

	class PositionOnlyDepthPSO : public RefCountedObject
	{
	public:
		PositionOnlyDepthPSO( D3D12CommandList* CommandList, CommonResources* CR );
		
		TRefCountPtr<GraphicsPipelineState> m_CullNonePSO;
		TRefCountPtr<GraphicsPipelineState> m_CullBackPSO;
	};

	class ResolveEditorSelectionPSO : public RefCountedObject
	{
	public:
		ResolveEditorSelectionPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class SpriteEditorPrimitivePSO : public RefCountedObject
	{
	public:
		SpriteEditorPrimitivePSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class SpriteHitProxyPSO : public RefCountedObject
	{
	public:
		SpriteHitProxyPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class LightPassPSO : public RefCountedObject
	{
	public:
		LightPassPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class DebugLineThicknessPSO : public RefCountedObject
	{
	public:
		DebugLineThicknessPSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

	class DebugLinePSO : public RefCountedObject
	{
	public:
		DebugLinePSO( D3D12CommandList* CommandList, CommonResources* CR );
		TRefCountPtr<GraphicsPipelineState> m_PSO;
	};

#if WITH_EDITOR
	class Texture2DToTextureCubePSO
	{
	public:

		Texture2DToTextureCubePSO( ID3D12GraphicsCommandList2* CommandList, ID3D12RootSignature* RS, DXGI_FORMAT Format);
		~Texture2DToTextureCubePSO();
		
		ID3D12PipelineState* m_PSO;
		ID3D12PipelineState* m_MipPSO;
	};

	class BufferVisualizerPSO : public RefCountedObject
	{
	public:

		BufferVisualizerPSO(D3D12CommandList* CommandList, CommonResources* CR);
		TRefCountPtr<GraphicsPipelineState> GetPSOForBufferVisualizer(EBufferVisualization BufferVialization);
		
		TRefCountPtr<GraphicsPipelineState> m_BaseColorPSO;
		TRefCountPtr<GraphicsPipelineState> m_MetallicPSO;
		TRefCountPtr<GraphicsPipelineState> m_RoughnessPSO;
		TRefCountPtr<GraphicsPipelineState> m_MaterialAoPSO;
		TRefCountPtr<GraphicsPipelineState> m_ShadingModelPSO;
		TRefCountPtr<GraphicsPipelineState> m_WorldNormalPSO;
		TRefCountPtr<GraphicsPipelineState> m_SubsurfaceColorPSO;
		TRefCountPtr<GraphicsPipelineState> m_DepthPSO;
		TRefCountPtr<GraphicsPipelineState> m_LinearDepthPSO;
		TRefCountPtr<GraphicsPipelineState> m_PreTonemapPSO;
		TRefCountPtr<GraphicsPipelineState> m_ScreenSpaceAOPSO;
		TRefCountPtr<GraphicsPipelineState> m_Bloom;
		TRefCountPtr<GraphicsPipelineState> m_ScreenSpaceReflection;
	};
#endif

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

		CommonResources( D3D12CommandList* CommandList );
		~CommonResources();

		static void Init( D3D12CommandList* CommandList );
		static void Shutdown();

		inline static CommonResources* Get() { return m_SingletonInstance; }

		TRefCountPtr<class VertexDeclaration> VertexDeclaration_Pos;
		TRefCountPtr<class VertexDeclaration> VertexDeclaration_PosUV;
		TRefCountPtr<class VertexDeclaration> VertexDeclaration_LineColorThickness;

		ScreenTriangle* m_ScreenTriangle;
		BackfaceScreenTriangle* m_BackfaceScreenTriangle;
		UniformQuad* m_UniformQuad;
		UniformCube* m_UniformCube;
		UniformCubePositionOnly* m_UniformCubePositionOnly;
		PointLightSphere* m_PointLightSphere;
		SpotLightCone* m_SpotLightCone;

		TRefCountPtr<ResolveAlphaBlendedPSO> m_ResolveAlphaBlendedPSO;
		TRefCountPtr<ResolveEditorSelectionPSO> m_ResolveEditorSelectionPSO;
		TRefCountPtr<TonemapPSO> m_TonemapPSO;
		TRefCountPtr<AmbientOcclusionPSO> m_AmbientOcclusionPSO;
		TRefCountPtr<LightPassPSO> m_LightPassPSO;
		TRefCountPtr<ScreenSpaceReflectionPSO> m_ScreenSpaceReflectionPSO;
		TRefCountPtr<ReflectionEnvironemntPSO> m_ReflectionEnvironmentPSO;
		TAAPSO* m_TAAPSO;
		TRefCountPtr<SceneDownSamplePSO> m_SceneDownSamplePSO;
		TRefCountPtr<BloomPSO> m_BloomPSO;
		TRefCountPtr<PositionOnlyDepthPSO> m_PositionOnlyDepthPSO;

		TRefCountPtr<SpriteEditorPrimitivePSO> m_SpriteEditorPrimitivePSO;
		TRefCountPtr<SpriteHitProxyPSO> m_SpriteHitProxyPSO;

		TRefCountPtr<DebugLineThicknessPSO> m_DebugLineThicknessPSO;
		TRefCountPtr<DebugLinePSO> m_DebugLinePSO;

		HZBPSO* m_HZBPSO;

		AssetHandle<Texture2D> m_SSAO_Random;
		AssetHandle<Texture2D> m_PreintegratedGF;

#if WITH_EDITOR
		TRefCountPtr<BufferVisualizerPSO> m_BufferVisualizerPSO;
		AssetHandle<Texture2D> m_AssetIcon_Default;
		AssetHandle<Texture2D> m_AssetIcon_Level;
		AssetHandle<Texture2D> m_AssetIcon_StaticMesh;
		AssetHandle<Texture2D> m_AssetIcon_Material;
		AssetHandle<Texture2D> m_AssetIcon_Texture2D;
		AssetHandle<Texture2D> m_AssetIcon_TextureVolume;
		AssetHandle<Texture2D> m_AssetIcon_TextureCube;
#endif

	private:

		static CommonResources* m_SingletonInstance;
	};

	void CompileShaderString(const std::wstring& ShaderPath, const char* EntryPoint, const char* Profile, ID3DBlob*& ShaderBlob, const D3D_SHADER_MACRO* Macros = NULL);
	bool CompileShader( const std::wstring& ShaderPath, const wchar_t* EntryPoint, const wchar_t* Profile, const std::vector<const wchar_t*>& Macros, ID3DBlob** ByteBlob );
}