#include "DrnPCH.h"
#include "CommonResources.h"

#include "Runtime/Renderer/RenderGeometeryHelper.h"
#include "Runtime/Renderer/RenderBuffer.h"

#define PAR_SHAPES_IMPLEMENTATION
#include "ThirdParty/par/par_shapes.h"

#include <dxcapi.h>

LOG_DEFINE_CATEGORY( LogCommonResources, "CommonResources" );

namespace Drn
{
	CommonResources* CommonResources::m_SingletonInstance = nullptr;

	D3D12_INPUT_ELEMENT_DESC InputElement_PosUV[2] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_PosUV = { InputElement_PosUV, _countof( InputElement_PosUV ) };

// --------------------------------------------------------------------------------------------------------------------------------------------

	D3D12_INPUT_ELEMENT_DESC InputElement_Pos[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_Pos = { InputElement_Pos, _countof( InputElement_Pos ) };

// --------------------------------------------------------------------------------------------------------------------------------------------

	D3D12_INPUT_ELEMENT_DESC InputElement_LineColorThickness[3] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "THICKNESS", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC VertexLayout_LineColorThickness = { InputElement_LineColorThickness, _countof( InputElement_LineColorThickness ) };

// --------------------------------------------------------------------------------------------------------------------------------------------

	CommonResources::CommonResources( D3D12CommandList* CommandList )
	{
		m_ScreenTriangle = new ScreenTriangle( CommandList );
		m_BackfaceScreenTriangle = new BackfaceScreenTriangle( CommandList );
		m_UniformQuad = new UniformQuad( CommandList );
		m_UniformCube = new UniformCube( CommandList );
		m_UniformCubePositionOnly = new UniformCubePositionOnly( CommandList );
		m_PointLightSphere = new PointLightSphere( CommandList );
		m_SpotLightCone = new SpotLightCone(CommandList);
		m_ResolveAlphaBlendedPSO = new ResolveAlphaBlendedPSO(CommandList->GetD3D12CommandList());
		m_ResolveEditorSelectionPSO = new ResolveEditorSelectionPSO(CommandList->GetD3D12CommandList());
		m_TonemapPSO = new TonemapPSO(CommandList->GetD3D12CommandList());
		m_AmbientOcclusionPSO = new AmbientOcclusionPSO(CommandList->GetD3D12CommandList());
		m_ScreenSpaceReflectionPSO = new ScreenSpaceReflectionPSO(CommandList->GetD3D12CommandList());
		m_ReflectionEnvironmentPSO = new ReflectionEnvironemntPSO(CommandList->GetD3D12CommandList());
		m_TAAPSO = new TAAPSO(CommandList->GetD3D12CommandList());
		m_SceneDownSamplePSO = new SceneDownSamplePSO(CommandList->GetD3D12CommandList());
		m_BloomPSO = new BloomPSO(CommandList->GetD3D12CommandList());
		m_PositionOnlyDepthPSO = new PositionOnlyDepthPSO(CommandList->GetD3D12CommandList());
		m_SpriteEditorPrimitivePSO = new SpriteEditorPrimitivePSO(CommandList->GetD3D12CommandList());
		m_SpriteHitProxyPSO = new SpriteHitProxyPSO(CommandList->GetD3D12CommandList());
		m_LightPassPSO = new LightPassPSO(CommandList->GetD3D12CommandList());
		m_DebugLineThicknessPSO = new DebugLineThicknessPSO(CommandList->GetD3D12CommandList());
		m_DebugLinePSO = new DebugLinePSO(CommandList->GetD3D12CommandList());
		m_HZBPSO = new HZBPSO(CommandList->GetD3D12CommandList());

		m_SSAO_Random = AssetHandle<Texture2D>( "Engine\\Content\\Textures\\SSAO_Jitter.drn" );
		m_SSAO_Random.Load();
		m_SSAO_Random->UploadResources(CommandList);

		m_PreintegratedGF = AssetHandle<Texture2D>( "Engine\\Content\\Textures\\T_IntegeratedGF.drn" );
		m_PreintegratedGF.Load();
		m_PreintegratedGF->UploadResources(CommandList);

#if WITH_EDITOR
		m_BufferVisualizerPSO = new BufferVisualizerPSO(CommandList->GetD3D12CommandList());

#define LOAD_TEXTURE( name , path )				\
	name = AssetHandle<Texture2D>(path);	\
	name.Load();							\
	name->UploadResources( CommandList );	

		LOAD_TEXTURE(m_AssetIcon_Default, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_Default.drn");
		LOAD_TEXTURE(m_AssetIcon_Level, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_Level.drn");
		LOAD_TEXTURE(m_AssetIcon_StaticMesh, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_StaticMesh.drn");
		LOAD_TEXTURE(m_AssetIcon_Material, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_Material.drn");
		LOAD_TEXTURE(m_AssetIcon_Texture2D, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_Texture2D.drn");
		LOAD_TEXTURE(m_AssetIcon_TextureVolume, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_TextureVolume.drn");
		LOAD_TEXTURE(m_AssetIcon_TextureCube, "Engine\\Content\\EditorResources\\AssetIcons\\T_AssetIcon_TextureCube.drn");

#undef LOAD_TEXTURE
#endif
	}

	CommonResources::~CommonResources()
	{
		delete m_ScreenTriangle;
		delete m_BackfaceScreenTriangle;
		delete m_UniformQuad;
		delete m_UniformCube;
		delete m_UniformCubePositionOnly;
		delete m_PointLightSphere;
		delete m_SpotLightCone;
		delete m_ResolveAlphaBlendedPSO;
		delete m_ResolveEditorSelectionPSO;
		delete m_TonemapPSO;
		delete m_AmbientOcclusionPSO;
		delete m_ScreenSpaceReflectionPSO;
		delete m_ReflectionEnvironmentPSO;
		delete m_TAAPSO;
		delete m_SceneDownSamplePSO;
		delete m_BloomPSO;
		delete m_PositionOnlyDepthPSO;
		delete m_SpriteEditorPrimitivePSO;
		delete m_SpriteHitProxyPSO;
		delete m_LightPassPSO;
		delete m_DebugLineThicknessPSO;
		delete m_DebugLinePSO;
		delete m_HZBPSO;

#if WITH_EDITOR
		delete m_BufferVisualizerPSO;
#endif
	}

	void CommonResources::Init( D3D12CommandList* CommandList )
	{
		if (!m_SingletonInstance)
		{
			m_SingletonInstance = new CommonResources( CommandList );
		}
	}

	void CommonResources::Shutdown()
	{
		if (m_SingletonInstance)
		{
			delete m_SingletonInstance;
			m_SingletonInstance = nullptr;
		}
	}

	float TriangleVertexData[] = 
	{
		1, -1, 0, 1, 1,
		-3, -1, 0, -1, 1,
		1, 3, 0, 1, -1
	};

	uint16 TriangleIndexData[] = { 0, 1, 2 };

	ScreenTriangle::ScreenTriangle( D3D12CommandList* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList->GetD3D12CommandList(), TriangleVertexData, 3, sizeof(TriangleVertexData) / 3, "ScreenTriangle");

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, TriangleIndexData, ClearValueBinding::Black, "IB_ScreenTriangle");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(TriangleIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	ScreenTriangle::~ScreenTriangle()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void ScreenTriangle::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);

		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	float TrianglePosVertexData[] = 
	{
		1, -1, 0,
		-3, -1, 0,
		1, 3, 0
	};

	uint16 TrianglePosIndexData[] = { 2, 1, 0 };

	BackfaceScreenTriangle::BackfaceScreenTriangle( D3D12CommandList* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList->GetD3D12CommandList(), TrianglePosVertexData, 3, sizeof(TrianglePosVertexData) / 3, "ScreenTriangleNoUV");

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, TrianglePosIndexData, ClearValueBinding::Black, "IB_ScreenTriangleNoUV");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(TrianglePosIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	BackfaceScreenTriangle::~BackfaceScreenTriangle()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void BackfaceScreenTriangle::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	float UniformQuadVertexData[] = 
	{
		0.5, -0.5, 0, 1, 1,
		-0.5, -0.5, 0, 0, 1,
		0.5, 0.5, 0, 1, 0,
		-0.5, 0.5, 0, 0, 0
	};

	uint16 UniformQuadIndexData[] = { 3, 1, 2, 2, 1, 0 };

	UniformQuad::UniformQuad( D3D12CommandList* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList->GetD3D12CommandList(), UniformQuadVertexData, 4, sizeof(UniformQuadVertexData) / 4, "UniformQuad");

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, UniformQuadVertexData, ClearValueBinding::Black, "IB_UniformQuad");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(UniformQuadVertexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	UniformQuad::~UniformQuad()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void UniformQuad::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	float UniformCubeVertexData[] = 
	{
		-1, -1, 1,		1, 1,		// 0
		1, -1, 1,		1, 1,		// 1
		-1, 1, 1,		1, 1,		// 2
		1, 1, 1,		1, 1,		// 3
		-1, -1, -1,		1, 1,		// 4
		1, -1, -1,		1, 1,		// 5
		-1, 1, -1,		1, 1,		// 6
		1, 1, -1,		1, 1		// 7
	};

	uint16 UniformCubeIndexData[] = {
		// top
		2, 6, 7,
		2, 3, 7,
		// bottom
		0, 4, 5,
		0, 1, 5,
		// left
		0, 2, 6,
		0, 4, 6,
		// right
		1, 3, 7,
		1, 5, 7,
		// front
		0, 2, 3,
		0, 1, 3,
		// back
		4, 6, 7,
		4, 5, 7
	};

	UniformCube::UniformCube( D3D12CommandList* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList->GetD3D12CommandList(), UniformCubeVertexData, 8, sizeof(UniformCubeVertexData) / 8, "UniformCube");

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, UniformCubeIndexData, ClearValueBinding::Black, "IB_UniformCube");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(UniformCubeIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	UniformCube::~UniformCube()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void UniformCube::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	float UniformCubePositionVertexData[] = 
	{
		-1, -1, 1,	// 0
		-1, 1, 1,	// 1
		-1, -1, -1,	// 2
		-1, 1, -1,	// 3
		1, -1, 1,	// 4
		1, 1, 1,	// 5
		1, -1, -1,	// 6
		1, 1, -1,	// 7
	};

	uint16 UniformCubePositionIndexData[] = {

		1, 2, 0,
		3, 6, 2,

		7, 4, 6,
		5, 0, 4,

		6, 0, 2,
		3, 5, 7,

		1, 3, 2,
		3, 7, 6,

		7, 5, 4,
		5, 1, 0,

		6, 4, 0,
		3, 1, 5
	};

	UniformCubePositionOnly::UniformCubePositionOnly( D3D12CommandList* CommandList )
	{
		m_VertexBuffer = VertexBuffer::Create(CommandList->GetD3D12CommandList(), UniformCubePositionVertexData, 8, sizeof(UniformCubePositionVertexData) / 8, "UniformCubePosition");

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, UniformCubePositionIndexData, ClearValueBinding::Black, "IB_UniformCubePosition");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(UniformCubePositionIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	UniformCubePositionOnly::~UniformCubePositionOnly()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void UniformCubePositionOnly::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	PointLightSphere::PointLightSphere( D3D12CommandList* CommandList )
	{
		par_shapes_mesh* SphereMesh = par_shapes_create_subdivided_sphere( 2 );

		const uint32 VertexCount = SphereMesh->npoints;
		const uint32 IndexCount = SphereMesh->ntriangles * 3;

		const uint32 IndexBufferSize = IndexCount * sizeof(PAR_SHAPES_T);

		m_VertexBuffer = VertexBuffer::Create(CommandList->GetD3D12CommandList(), SphereMesh->points, VertexCount, sizeof(float) * 3, "PointLightSphere");

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, SphereMesh->triangles, ClearValueBinding::Black, "IB_PointLightSphere");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), IndexBufferSize, IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);

		par_shapes_free_mesh(SphereMesh);
	}

	PointLightSphere::~PointLightSphere()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void PointLightSphere::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	SpotLightCone::SpotLightCone( D3D12CommandList* CommandList )
	{
		RenderIndexBuffer* IB;
		RenderGeometeryHelper::CreateSpotlightStencilGeometery(CommandList, m_VertexBuffer, IB);
		m_IndexBuffer = IB;
	}

	SpotLightCone::~SpotLightCone()
	{
		if (m_VertexBuffer) { delete m_VertexBuffer; }
	}

	void SpotLightCone::BindAndDraw( D3D12CommandList* CommandList )
	{
		m_VertexBuffer->Bind(CommandList->GetD3D12CommandList());
		CommandList->SetIndexBuffer(m_IndexBuffer->m_ResourceLocation, m_IndexBuffer->GetStride() == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
		CommandList->GetD3D12CommandList()->DrawIndexedInstanced(m_IndexBuffer->GetSize() / m_IndexBuffer->GetStride(), 1, 0, 0, 0);
	}

// --------------------------------------------------------------------------------------

	ResolveAlphaBlendedPSO::ResolveAlphaBlendedPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws(Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveAlphaBlended.hlsl" ));
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros , &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros , &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_ResolveAlphaBlended");
#endif
	}

	ResolveAlphaBlendedPSO::~ResolveAlphaBlendedPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	ResolveEditorSelectionPSO::ResolveEditorSelectionPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveEditorSelection.hlsl" ) );
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_ResolveEditorSelection");
#endif
	}

	ResolveEditorSelectionPSO::~ResolveEditorSelectionPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	TonemapPSO::TonemapPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Tonemap.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DISPLAY_OUTPUT_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_Tonemap");
#endif
	}

	TonemapPSO::~TonemapPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	AmbientOcclusionPSO::AmbientOcclusionPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_SetupPSO = nullptr;
		m_HalfPSO = nullptr;
		m_MainPSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceAmbientOcclusionSetup.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = {};
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R16G16B16A16_FLOAT;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_SetupPSO ) );
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceAmbientOcclusion.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { L"USE_AO_SETUP_AS_INPUT=1" };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8_UNORM;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_HalfPSO ) );
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceAmbientOcclusion.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = {};
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.DSVFormat							= DXGI_FORMAT_R32_FLOAT;
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8_UNORM;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_MainPSO ) );
		}

#if D3D12_Debug_INFO
		m_SetupPSO->SetName(L"PSO_SetupAO");
		m_HalfPSO->SetName(L"HalfPSO_AO");
		m_MainPSO->SetName(L"PSO_AO");
#endif
	}

	AmbientOcclusionPSO::~AmbientOcclusionPSO()
	{
		m_SetupPSO->Release();
		m_HalfPSO->Release();
		m_MainPSO->Release();
	}

// --------------------------------------------------------------------------------------

	ScreenSpaceReflectionPSO::ScreenSpaceReflectionPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceReflection.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R16G16B16A16_FLOAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_ScreenSpaceReflection");
#endif
	}

	ScreenSpaceReflectionPSO::~ScreenSpaceReflectionPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	ReflectionEnvironemntPSO::ReflectionEnvironemntPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ReflectionEnvironment.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R16G16B16A16_FLOAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_ReflectionEnvironemnt");
#endif
	}

	ReflectionEnvironemntPSO::~ReflectionEnvironemntPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	TAAPSO::TAAPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\TemporalAA.hlsl" ) );
		ID3DBlob* ComputeShaderBlob;
		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

		D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.CS									= CD3DX12_SHADER_BYTECODE(ComputeShaderBlob);

		Device->CreateComputePipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_TAA");
#endif
	}

	TAAPSO::~TAAPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	SceneDownSamplePSO::SceneDownSamplePSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\SceneDownSample.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= SCENE_DOWN_SAMPLE_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_SceneDownSample");
#endif
	}

	SceneDownSamplePSO::~SceneDownSamplePSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	BloomPSO::BloomPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_BloomYPSO = nullptr;
		m_BloomXPSO = nullptr;
		m_BloomXAddtivePSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\GaussianBloom.hlsl" ) );

		const std::wstring StaticSampleMacro = std::wstring(L"STATIC_SAMPLE_COUNT=") + std::to_wstring(BLOOM_STATIC_SAMPLE_COUNT);

		{
			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { StaticSampleMacro.c_str(), L"BLOOM_Y=1" };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= BLOOM_FORMAT;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_BloomYPSO ) );

#if D3D12_Debug_INFO
			m_BloomYPSO->SetName(L"PSO_BloomY");
#endif
		}

		{
			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { StaticSampleMacro.c_str() };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= BLOOM_FORMAT;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_BloomXPSO ) );

#if D3D12_Debug_INFO
			m_BloomXPSO->SetName(L"PSO_BloomX");
#endif
		}

		{
			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { StaticSampleMacro.c_str(), L"BLOOM_ADDTIVE=1" };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= BLOOM_FORMAT;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_BloomXAddtivePSO ) );

#if D3D12_Debug_INFO
			m_BloomXAddtivePSO->SetName(L"PSO_BloomXAddtive");
#endif
		}
	}

	BloomPSO::~BloomPSO()
	{
		m_BloomYPSO->Release();
		m_BloomXPSO->Release();
		m_BloomXAddtivePSO->Release();
	}

// --------------------------------------------------------------------------------------

	PositionOnlyDepthPSO::PositionOnlyDepthPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_CullNonePSO = nullptr;
		m_CullBackPSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\PositionOnlyDepthVertexShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);

		D3D12_RASTERIZER_DESC RasterizerDesc = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= InputLayout::GetLayoutDescriptionForType(EInputLayoutType::Position);
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.NumRenderTargets					= 0;
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_CullNonePSO ) );

#if D3D12_Debug_INFO
		m_CullNonePSO->SetName(L"PSO_PositionOnlyDepth_CullNone");
#endif

		{
			RasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
			PipelineDesc.RasterizerState = RasterizerDesc;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_CullBackPSO ) );

#if D3D12_Debug_INFO
			m_CullBackPSO->SetName(L"PSO_PositionOnlyDepth_CullBack");
#endif
		}
	}

	PositionOnlyDepthPSO::~PositionOnlyDepthPSO()
	{
		m_CullNonePSO->Release();
		m_CullBackPSO->Release();
	}

// --------------------------------------------------------------------------------------

	SpriteEditorPrimitivePSO::SpriteEditorPrimitivePSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Sprite.hlsl" ) );
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		//PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_SpriteEditorPrimitive");
#endif
	}

	SpriteEditorPrimitivePSO::~SpriteEditorPrimitivePSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	SpriteHitProxyPSO::SpriteHitProxyPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Sprite.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {L"HITPROXY_PASS=1"};

		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		//PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_GUID_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_SpriteHitProxy");
#endif
	}

	SpriteHitProxyPSO::~SpriteHitProxyPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	LightPassPSO::LightPassPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\LightPassDeferred.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::wstring SideMacro = StringHelper::s2ws("SPOTLIGHT_STENCIL_SIDES=" + std::to_string(SPOTLIGHT_STENCIL_SIDES));
		std::wstring SliceMacro = StringHelper::s2ws("SPOTLIGHT_STENCIL_SLICES=" + std::to_string(SPOTLIGHT_STENCIL_SLICES));

		const std::vector<const wchar_t*> Macros = { SideMacro.c_str(), SliceMacro.c_str() };
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_BLEND_DESC BlendDesc = {};
		BlendDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
		RasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_Pos;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= BlendDesc;
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= GBUFFER_COLOR_DEFERRED_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_LightpassDeferred");
#endif
	}

	LightPassPSO::~LightPassPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	DebugLineThicknessPSO::DebugLineThicknessPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\DebugLineThicknessShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;
		ID3DBlob* GeometeryShaderBlob;

		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);
		CompileShader( ShaderPath, L"Main_GS", L"gs_6_6", Macros, &GeometeryShaderBlob);

		CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
		RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_LineColorThickness;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.GS									= CD3DX12_SHADER_BYTECODE(GeometeryShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DISPLAY_OUTPUT_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_DebugLineThickness");
#endif
	}

	DebugLineThicknessPSO::~DebugLineThicknessPSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	DebugLinePSO::DebugLinePSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\DebugLineShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
		RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_LineColorThickness;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		PipelineDesc.RasterizerState					= RasterizerDesc;
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		PipelineDesc.DepthStencilState.DepthEnable		= TRUE;
		PipelineDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;
		PipelineDesc.DepthStencilState.DepthFunc		= D3D12_COMPARISON_FUNC_GREATER;
		PipelineDesc.DepthStencilState.StencilEnable	= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.DSVFormat							= DEPTH_FORMAT;
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DISPLAY_OUTPUT_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
		m_PSO->SetName(L"PSO_DebugLine");
#endif
	}

	DebugLinePSO::~DebugLinePSO()
	{
		m_PSO->Release();
	}

// --------------------------------------------------------------------------------------

#if WITH_EDITOR
	Texture2DToTextureCubePSO::Texture2DToTextureCubePSO( ID3D12GraphicsCommandList2* CommandList, ID3D12RootSignature* RS, DXGI_FORMAT Format)
	{
		m_PSO = nullptr;
		m_MipPSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Texture2DToTextureCube.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;
			ID3DBlob* GeometeryShaderBlob;

			const std::vector<const wchar_t*> Macros;
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);
			CompileShader( ShaderPath, L"Main_GS", L"gs_6_6", Macros, &GeometeryShaderBlob);

			CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
			RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= RS;
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= RasterizerDesc;
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.GS									= CD3DX12_SHADER_BYTECODE(GeometeryShaderBlob);
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= Format;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_PSO ) );

#if D3D12_Debug_INFO
			m_PSO->SetName(L"PSO_Texture2DToTextureCube");
#endif
		}

// -----------------------------------------------------------------------------------------------------------------------

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Texture2DToTextureCube.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;
			ID3DBlob* GeometeryShaderBlob;

			const std::vector<const wchar_t*> Macros;
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Mip_PS", L"ps_6_6", Macros, &PixelShaderBlob);
			CompileShader( ShaderPath, L"Main_GS", L"gs_6_6", Macros, &GeometeryShaderBlob);

			CD3DX12_RASTERIZER_DESC RasterizerDesc(D3D12_DEFAULT);
			RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= RS;
			PipelineDesc.InputLayout						= VertexLayout_PosUV;
			PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.RasterizerState					= RasterizerDesc;
			PipelineDesc.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
			PipelineDesc.SampleMask							= UINT_MAX;
			PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
			PipelineDesc.GS									= CD3DX12_SHADER_BYTECODE(GeometeryShaderBlob);
			PipelineDesc.NumRenderTargets					= 1;
			PipelineDesc.RTVFormats[0]						= Format;
			PipelineDesc.SampleDesc.Count					= 1;

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_MipPSO ) );

#if D3D12_Debug_INFO
			m_MipPSO->SetName(L"PSO_Texture2DToTextureCubeMip");
#endif
		}
	}

	Texture2DToTextureCubePSO::~Texture2DToTextureCubePSO()
	{
		m_PSO->Release();
		m_MipPSO->Release();
	}

// --------------------------------------------------------------------------------------

	BufferVisualizerPSO::BufferVisualizerPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_BaseColorPSO = nullptr;
		m_MetallicPSO = nullptr;
		m_RoughnessPSO = nullptr;
		m_MaterialAoPSO = nullptr;
		m_ShadingModelPSO = nullptr;
		m_WorldNormalPSO = nullptr;
		m_SubsurfaceColorPSO = nullptr;
		m_DepthPSO = nullptr;
		m_LinearDepthPSO = nullptr;
		m_PreTonemapPSO = nullptr;
		m_ScreenSpaceAOPSO = nullptr;
		m_Bloom = nullptr;
		m_ScreenSpaceReflection = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\BufferVisualizer.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = { L"BASECOLOR=1" };
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};
		PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
		PipelineDesc.InputLayout						= VertexLayout_PosUV;
		PipelineDesc.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PipelineDesc.RasterizerState					= CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
		PipelineDesc.BlendState							= CD3DX12_BLEND_DESC ( D3D12_DEFAULT );
		PipelineDesc.DepthStencilState.DepthEnable		= FALSE;
		PipelineDesc.SampleMask							= UINT_MAX;
		PipelineDesc.VS									= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
		PipelineDesc.PS									= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);
		PipelineDesc.NumRenderTargets					= 1;
		PipelineDesc.RTVFormats[0]						= DISPLAY_OUTPUT_FORMAT;
		PipelineDesc.SampleDesc.Count					= 1;

		Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &m_BaseColorPSO ) );

		auto CreateVisualizerPSO = [&](std::vector<const wchar_t*> InMacros, ID3D12PipelineState*& PSO)
		{
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", InMacros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", InMacros, &PixelShaderBlob);

			PipelineDesc.VS	= CD3DX12_SHADER_BYTECODE(VertexShaderBlob);
			PipelineDesc.PS	= CD3DX12_SHADER_BYTECODE(PixelShaderBlob);

			Device->CreateGraphicsPipelineState( &PipelineDesc, IID_PPV_ARGS( &PSO ) );
		};

		CreateVisualizerPSO({ L"METALLIC=1" }, m_MetallicPSO);
		CreateVisualizerPSO({ L"ROUGHNESS=1" }, m_RoughnessPSO);
		CreateVisualizerPSO({ L"MATERIAL_AO=1" }, m_MaterialAoPSO);
		CreateVisualizerPSO({ L"SHADING_MODEL=1" }, m_ShadingModelPSO);
		CreateVisualizerPSO({ L"WORLD_NORMAL=1" }, m_WorldNormalPSO);
		CreateVisualizerPSO({ L"SUBSURFACE_COLOR=1" }, m_SubsurfaceColorPSO);
		CreateVisualizerPSO({ L"DEPTH=1" }, m_DepthPSO);
		CreateVisualizerPSO({ L"LINEAR_DEPTH=1" }, m_LinearDepthPSO);
		CreateVisualizerPSO({ L"PRE_TONEMAP=1" }, m_PreTonemapPSO);
		CreateVisualizerPSO({ L"SCREEN_SPACE_AO=1" }, m_ScreenSpaceAOPSO);
		CreateVisualizerPSO({ L"BLOOM=1" }, m_Bloom);
		CreateVisualizerPSO({ L"SCREEN_SPACE_REFLECTION=1" }, m_ScreenSpaceReflection);

#if D3D12_Debug_INFO
		m_BaseColorPSO->SetName(L"PSO_BufferVisualizer_BaseColor");
#endif
	}

	BufferVisualizerPSO::~BufferVisualizerPSO()
	{
		m_BaseColorPSO->Release();
		m_MetallicPSO->Release();
		m_RoughnessPSO->Release();
		m_MaterialAoPSO->Release();
		m_ShadingModelPSO->Release();
		m_WorldNormalPSO->Release();
		m_SubsurfaceColorPSO->Release();
		m_DepthPSO->Release();
		m_LinearDepthPSO->Release();
		m_PreTonemapPSO->Release();
		m_ScreenSpaceAOPSO->Release();
		m_Bloom->Release();
		m_ScreenSpaceReflection->Release();
	}

	ID3D12PipelineState* BufferVisualizerPSO::GetPSOForBufferVisualizer( EBufferVisualization BufferVialization )
	{
		switch ( BufferVialization )
		{
		case EBufferVisualization::BaseColor:				return m_BaseColorPSO;
		case EBufferVisualization::Metallic:				return m_MetallicPSO;
		case EBufferVisualization::Roughness:				return m_RoughnessPSO;
		case EBufferVisualization::MaterialAO:				return m_MaterialAoPSO;
		case EBufferVisualization::ShadingModel:			return m_ShadingModelPSO;
		case EBufferVisualization::WorldNormal:				return m_WorldNormalPSO;
		case EBufferVisualization::SubsurfaceColor:			return m_SubsurfaceColorPSO;
		case EBufferVisualization::Depth:					return m_DepthPSO;
		case EBufferVisualization::LinearDepth:				return m_LinearDepthPSO;
		case EBufferVisualization::PreTonemapColor:			return m_PreTonemapPSO;
		case EBufferVisualization::ScreenSpaceAO:			return m_ScreenSpaceAOPSO;
		case EBufferVisualization::Bloom:					return m_Bloom;
		case EBufferVisualization::ScreenSpaceReflection:	return m_ScreenSpaceReflection;
		case EBufferVisualization::FinalImage:
		default: return nullptr;
		}
	}

#endif

// --------------------------------------------------------------------------------------

	HZBPSO::HZBPSO( ID3D12GraphicsCommandList2* CommandList )
	{
		m_1Mip_PSO = nullptr;
		m_2Mip_PSO = nullptr;
		m_3Mip_PSO = nullptr;
		m_4Mip_PSO = nullptr;

		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=1"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.CS									= CD3DX12_SHADER_BYTECODE(ComputeShaderBlob);

			Device->CreateComputePipelineState( &PipelineDesc, IID_PPV_ARGS( &m_1Mip_PSO ) );
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=2"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.CS									= CD3DX12_SHADER_BYTECODE(ComputeShaderBlob);

			Device->CreateComputePipelineState( &PipelineDesc, IID_PPV_ARGS( &m_2Mip_PSO ) );
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=3"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.CS									= CD3DX12_SHADER_BYTECODE(ComputeShaderBlob);

			Device->CreateComputePipelineState( &PipelineDesc, IID_PPV_ARGS( &m_3Mip_PSO ) );
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=4"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			D3D12_COMPUTE_PIPELINE_STATE_DESC PipelineDesc = {};
			PipelineDesc.pRootSignature						= Renderer::Get()->m_BindlessRootSinature.Get();
			PipelineDesc.CS									= CD3DX12_SHADER_BYTECODE(ComputeShaderBlob);

			Device->CreateComputePipelineState( &PipelineDesc, IID_PPV_ARGS( &m_4Mip_PSO ) );
		}

#if D3D12_Debug_INFO
		m_1Mip_PSO->SetName(L"PSO_HZB_1Mip");
		m_2Mip_PSO->SetName(L"PSO_HZB_2Mip");
		m_3Mip_PSO->SetName(L"PSO_HZB_3Mip");
		m_4Mip_PSO->SetName(L"PSO_HZB_4Mip");
#endif
	}

	HZBPSO::~HZBPSO()
	{
		m_1Mip_PSO->Release();
		m_2Mip_PSO->Release();
		m_3Mip_PSO->Release();
		m_4Mip_PSO->Release();
	}

// --------------------------------------------------------------------------------------

	void CompileShaderString( const std::wstring& ShaderPath, const char* EntryPoint, const char* Profile, ID3DBlob*& ShaderBlob, const D3D_SHADER_MACRO* Macros)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		flags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompileFromFile( ShaderPath.c_str(), Macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
			EntryPoint, Profile, flags, 0, &ShaderBlob, &errorBlob );

		if ( FAILED(hr) )
		{
			if ( errorBlob )
			{
				LOG(LogCommonResources, Error, "Shader compile failed. %s", (char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}

			if ( ShaderBlob )
				ShaderBlob->Release();
		}
	}

	bool CompileShader( const std::wstring& ShaderPath, const wchar_t* EntryPoint, const wchar_t* Profile, const std::vector<const wchar_t*>& Macros, ID3DBlob** ByteBlob )
	{
		Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
		Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;

		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.GetAddressOf()));

		Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
		pUtils->CreateDefaultIncludeHandler(pIncludeHandler.GetAddressOf());

		std::vector<LPCWSTR> Args;
		Args.push_back(L"Name");

		Args.push_back(L"-E");
		Args.push_back(EntryPoint);

		Args.push_back(L"-T");
		Args.push_back(Profile);

		Args.push_back(L"-Zs");

		Args.push_back( L"-O3" );

		Args.push_back(L"-I ..\\..\\..\\Engine\\Content\\Materials\\");

		for (auto& Mac : Macros)
		{
			Args.push_back(L"-D");
			Args.push_back(Mac);
		}

		//Args.push_back(L"-Fo");
		//Args.push_back( L"myshader.bin");
		//
		//Args.push_back(L"-Fd");
		//Args.push_back(L"myshader.pdb");

		Args.push_back(L"-Qstrip_reflect");

		Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource = nullptr;
		pUtils->LoadFile(ShaderPath.c_str(), nullptr, &pSource);
		DxcBuffer Source;
		Source.Ptr = pSource->GetBufferPointer();
		Source.Size = pSource->GetBufferSize();
		Source.Encoding = DXC_CP_ACP;

		Microsoft::WRL::ComPtr<IDxcResult> pResults;
		pCompiler->Compile(
			&Source,
			Args.data(),
			Args.size(),
			pIncludeHandler.Get(),
			IID_PPV_ARGS(pResults.GetAddressOf())
		);

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrors = nullptr;
		pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
		if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		{
			LOG(LogCommonResources, Warning, "\"%ws\" complation log:\n%s", ShaderPath.c_str(), pErrors->GetStringPointer());
		}

		HRESULT hrStatus;
		pResults->GetStatus(&hrStatus);
		if (FAILED(hrStatus))
		{
			LOG(LogCommonResources, Error, "shader complation Failed.");
			return false;
		}

		Microsoft::WRL::ComPtr<IDxcBlob> pShader = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName = nullptr;
		pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);

		D3DCreateBlob(pShader->GetBufferSize(), ByteBlob);
		memcpy((*ByteBlob)->GetBufferPointer(), pShader->GetBufferPointer(), pShader->GetBufferSize());

		return true;
	}


 }  // namespace Drn