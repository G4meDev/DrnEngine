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

	template<typename T>
	struct TriangleIndexList
	{
		TriangleIndexList(T A, T B, T C)
			: Data{A, B, C}
		{}
	
		TriangleIndexList()
			: TriangleIndexList(0, 1, 2)
		{}
	
		T Data[3];
	};

	typedef TriangleIndexList<uint16> TriangleIndexList_16;
	typedef TriangleIndexList<uint32> TriangleIndexList_32;

// --------------------------------------------------------------------------------------------------------------------------------------------

	struct PositionUV
	{
		PositionUV(float X, float Y, float Z, float U, float V)
			: Position{X, Y, Z}
			, UV{U, V}
		{}
	
		PositionUV()
			: PositionUV(0, 0, 0, 0, 0)
		{}

		float Position[3];
		float UV[2];
	};

// --------------------------------------------------------------------------------------------------------------------------------------------

	CommonResources::CommonResources( D3D12CommandList* CommandList )
	{
		VertexDeclaration_Pos = VertexDeclaration::Create(
		{
			VertexElement(0, 0, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0, 12)
		});

		VertexDeclaration_PosUV = VertexDeclaration::Create(
		{
			VertexElement(0, 0, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0, 20),
			VertexElement(0, 12, DXGI_FORMAT_R32G32_FLOAT, "TEXCOORD", 0, 20),
		});

		VertexDeclaration_LineColorThickness = VertexDeclaration::Create(
		{
			VertexElement(0, 0, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0, 20),
			VertexElement(0, 12, DXGI_FORMAT_R8G8B8A8_UNORM, "COLOR", 0, 20),
			VertexElement(0, 16, DXGI_FORMAT_R32_FLOAT, "THICKNESS", 0, 20),
		});

		VertexDeclaration_StaticMesh = VertexDeclaration::Create(
		{
			VertexElement(0, 0, DXGI_FORMAT_R32G32B32_FLOAT, "POSITION", 0, 12),
			VertexElement(1, 0, DXGI_FORMAT_R8G8B8A8_UNORM, "COLOR", 0, 4),
			VertexElement(2, 0, DXGI_FORMAT_R8G8B8A8_SNORM, "NORMAL", 0, 4),
			VertexElement(3, 0, DXGI_FORMAT_R8G8B8A8_SNORM, "TANGENT", 0, 4),
			VertexElement(4, 0, DXGI_FORMAT_R8G8B8A8_SNORM, "BINORMAL", 0, 4),
			VertexElement(5, 0, DXGI_FORMAT_R16G16_FLOAT, "TEXCOORD", 0, 4),
			VertexElement(6, 0, DXGI_FORMAT_R16G16_FLOAT, "TEXCOORD", 1, 4),
			VertexElement(7, 0, DXGI_FORMAT_R16G16_FLOAT, "TEXCOORD", 2, 4),
			VertexElement(8, 0, DXGI_FORMAT_R16G16_FLOAT, "TEXCOORD", 3, 4),
		});

		m_ScreenTriangle = new ScreenTriangle( CommandList );
		m_BackfaceScreenTriangle = new BackfaceScreenTriangle( CommandList );
		m_UniformQuad = new UniformQuad( CommandList );
		m_UniformCube = new UniformCube( CommandList );
		m_UniformCubePositionOnly = new UniformCubePositionOnly( CommandList );
		m_PointLightSphere = new PointLightSphere( CommandList );
		m_SpotLightCone = new SpotLightCone(CommandList);
		m_ResolveAlphaBlendedPSO = new ResolveAlphaBlendedPSO(CommandList, this);
		m_ResolveEditorSelectionPSO = new ResolveEditorSelectionPSO(CommandList, this);
		m_TonemapPSO = new TonemapPSO(CommandList, this);
		m_AmbientOcclusionPSO = new AmbientOcclusionPSO(CommandList, this);
		m_ScreenSpaceReflectionPSO = new ScreenSpaceReflectionPSO(CommandList, this);
		m_ReflectionEnvironmentPSO = new ReflectionEnvironemntPSO(CommandList, this);
		m_TAAPSO = new TAAPSO(CommandList);
		m_SceneDownSamplePSO = new SceneDownSamplePSO(CommandList, this);
		m_BloomPSO = new BloomPSO(CommandList, this);
		m_PositionOnlyDepthPSO = new PositionOnlyDepthPSO(CommandList, this);
		m_SpriteEditorPrimitivePSO = new SpriteEditorPrimitivePSO(CommandList, this);
		m_SpriteHitProxyPSO = new SpriteHitProxyPSO(CommandList, this);
		m_LightPassPSO = new LightPassPSO(CommandList, this);
		m_DebugLineThicknessPSO = new DebugLineThicknessPSO(CommandList, this);
		m_DebugLinePSO = new DebugLinePSO(CommandList, this);
		m_HZBPSO = new HZBPSO(CommandList);

		m_SSAO_Random = AssetHandle<Texture2D>( "Engine\\Content\\Textures\\SSAO_Jitter.drn" );
		m_SSAO_Random.Load();
		m_SSAO_Random->UploadResources(CommandList);

		m_PreintegratedGF = AssetHandle<Texture2D>( "Engine\\Content\\Textures\\T_IntegeratedGF.drn" );
		m_PreintegratedGF.Load();
		m_PreintegratedGF->UploadResources(CommandList);

#if WITH_EDITOR
		m_BufferVisualizerPSO = new BufferVisualizerPSO(CommandList, this);

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

	PositionUV TriangleVertexData[] = 
	{
		{ 1, -1, 0,		 1,  1},
		{-3, -1, 0,		-1,  1},
		{ 1,  3, 0,		 1, -1}
	};

	ScreenTriangle::ScreenTriangle( D3D12CommandList* CommandList )
	{
		uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, TriangleVertexData, ClearValueBinding::Black, "VB_ScreenTriangle");
		m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(TriangleVertexData), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
	}

	ScreenTriangle::~ScreenTriangle()
	{}

	void ScreenTriangle::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(PositionUV) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawPrimitive(0, 1, 1);
	}

// --------------------------------------------------------------------------------------

	Vector TrianglePosVertexData[] = 
	{
		{ 1,  3, 0},
		{-3, -1, 0},
		{ 1, -1, 0}
	};

	BackfaceScreenTriangle::BackfaceScreenTriangle( D3D12CommandList* CommandList )
	{
		uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, TrianglePosVertexData, ClearValueBinding::Black, "VB_ScreenTriangleNoUV");
		m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(TrianglePosVertexData), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);
	}

	BackfaceScreenTriangle::~BackfaceScreenTriangle()
	{}

	void BackfaceScreenTriangle::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(Vector) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawPrimitive(0, 1, 1);
	}

// --------------------------------------------------------------------------------------

	PositionUV UniformQuadVertexData[] = 
	{
		{ 0.5, -0.5, 0,		1, 1},
		{-0.5, -0.5, 0,		0, 1},
		{ 0.5,  0.5, 0,		1, 0},
		{-0.5,  0.5, 0,		0, 0}
	};

	TriangleIndexList_16 UniformQuadIndexData[] =
	{
		{3, 1, 2},
		{2, 1, 0}
	};

	UniformQuad::UniformQuad( D3D12CommandList* CommandList )
	{
		VertexCount = _countof(UniformQuadVertexData);
		PrimitiveCount = _countof(UniformQuadIndexData);

		uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, UniformQuadVertexData, ClearValueBinding::Black, "VB_UniformQuad");
		m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(UniformQuadVertexData), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, UniformQuadIndexData, ClearValueBinding::Black, "IB_UniformQuad");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(UniformQuadIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	UniformQuad::~UniformQuad()
	{}

	void UniformQuad::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(PositionUV) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawIndexedPrimitive(m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, 1);
	}

// --------------------------------------------------------------------------------------

	PositionUV UniformCubeVertexData[] = 
	{
		{-1, -1,  1,	1, 1},		// 0
		{ 1, -1,  1,	1, 1},		// 1
		{-1,  1,  1,	1, 1},		// 2
		{ 1,  1,  1,	1, 1},		// 3
		{-1, -1, -1,	1, 1},		// 4
		{ 1, -1, -1,	1, 1},		// 5
		{-1,  1, -1,	1, 1},		// 6
		{ 1,  1, -1,	1, 1}		// 7
	};
	
	TriangleIndexList_16 UniformCubeIndexData[] = {
		// top
		{2, 6, 7},
		{2, 3, 7},
		// bottom
		{0, 4, 5},
		{0, 1, 5},
		// left
		{0, 2, 6},
		{0, 4, 6},
		// right
		{1, 3, 7},
		{1, 5, 7},
		// front
		{0, 2, 3},
		{0, 1, 3},
		// back
		{4, 6, 7},
		{4, 5, 7}
	};

	UniformCube::UniformCube( D3D12CommandList* CommandList )
	{
		VertexCount = _countof(UniformCubeVertexData);
		PrimitiveCount = _countof(UniformCubeIndexData);

		uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, UniformCubeVertexData, ClearValueBinding::Black, "VB_UniformCube");
		m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(UniformCubeVertexData), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, UniformCubeIndexData, ClearValueBinding::Black, "IB_UniformCube");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(UniformCubeIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	UniformCube::~UniformCube()
	{}

	void UniformCube::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(PositionUV) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawIndexedPrimitive(m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, 1);
	}

// --------------------------------------------------------------------------------------

	Vector UniformCubePositionVertexData[] = 
	{
		{-1, -1,  1	},	// 0
		{-1,  1,  1	},	// 1
		{-1, -1, -1	},	// 2
		{-1,  1, -1	},	// 3
		{ 1, -1,  1	},	// 4
		{ 1,  1,  1	},	// 5
		{ 1, -1, -1	},	// 6
		{ 1,  1, -1	},	// 7
	};

	TriangleIndexList_16 UniformCubePositionIndexData[] = 
	{
		{1, 2, 0},
		{3, 6, 2},
	
		{7, 4, 6},
		{5, 0, 4},
	
		{6, 0, 2},
		{3, 5, 7},
	
		{1, 3, 2},
		{3, 7, 6},
	
		{7, 5, 4},
		{5, 1, 0},
	
		{6, 4, 0},
		{3, 1, 5}
	};

	UniformCubePositionOnly::UniformCubePositionOnly( D3D12CommandList* CommandList )
	{
		VertexCount = _countof(UniformCubePositionVertexData);
		PrimitiveCount = _countof(UniformCubePositionIndexData);

		uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, UniformCubePositionVertexData, ClearValueBinding::Black, "VB_UniformCubePosition");
		m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(UniformCubePositionVertexData), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, UniformCubePositionIndexData, ClearValueBinding::Black, "IB_UniformCubePosition");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), sizeof(UniformCubePositionIndexData), IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);
	}

	UniformCubePositionOnly::~UniformCubePositionOnly()
	{}

	void UniformCubePositionOnly::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(Vector) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawIndexedPrimitive(m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, 1);
	}

// --------------------------------------------------------------------------------------

	PointLightSphere::PointLightSphere( D3D12CommandList* CommandList )
	{
		par_shapes_mesh* SphereMesh = par_shapes_create_subdivided_sphere( 2 );

		VertexCount = SphereMesh->npoints;
		PrimitiveCount = SphereMesh->ntriangles;

		const uint32 IndexCount = PrimitiveCount * 3;
		const uint32 IndexBufferSize = IndexCount * sizeof(PAR_SHAPES_T);

		uint32 VertexBufferFlags = (uint32)EBufferUsageFlags::VertexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo VertexBufferCreateInfo(nullptr, SphereMesh->points, ClearValueBinding::Black, "VB_PointLightSphere");
		m_VertexBuffer = RenderVertexBuffer::Create(CommandList->GetParentDevice(), CommandList, VertexCount * sizeof(Vector), VertexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, VertexBufferCreateInfo);

		uint32 IndexBufferFlags = (uint32)EBufferUsageFlags::IndexBuffer | (uint32)EBufferUsageFlags::Static;
		RenderResourceCreateInfo IndexBufferCreateInfo(nullptr, SphereMesh->triangles, ClearValueBinding::Black, "IB_PointLightSphere");
		m_IndexBuffer = RenderIndexBuffer::Create(CommandList->GetParentDevice(), CommandList, sizeof(uint16), IndexBufferSize, IndexBufferFlags, D3D12_RESOURCE_STATE_COMMON, false, IndexBufferCreateInfo);

		par_shapes_free_mesh(SphereMesh);
	}

	PointLightSphere::~PointLightSphere()
	{}

	void PointLightSphere::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(Vector) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawIndexedPrimitive(m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, 1);
	}

// --------------------------------------------------------------------------------------

	SpotLightCone::SpotLightCone( D3D12CommandList* CommandList )
	{
		RenderGeometeryHelper::CreateSpotlightStencilGeometery(CommandList, *m_VertexBuffer.GetInitReference(), *m_IndexBuffer.GetInitReference(), VertexCount, PrimitiveCount);
	}

	SpotLightCone::~SpotLightCone()
	{}

	void SpotLightCone::BindAndDraw( D3D12CommandList* CommandList )
	{
		uint16 const Strides[] = { sizeof(Vector) };
		CommandList->SetStreamSource(0, m_VertexBuffer, 0);
		CommandList->DrawIndexedPrimitive(m_IndexBuffer, 0, 0, VertexCount, 0, PrimitiveCount, 1);
	}

// --------------------------------------------------------------------------------------

	ResolveAlphaBlendedPSO::ResolveAlphaBlendedPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws(Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveAlphaBlended.hlsl" ));
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;
		
		std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros , &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros , &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		BlendStateInitializer BInit = {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::One)};
		TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);

		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_ResolveAlphaBlended");
	}

// --------------------------------------------------------------------------------------

	ResolveEditorSelectionPSO::ResolveEditorSelectionPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ResolveEditorSelection.hlsl" ) );
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);


		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		BlendStateInitializer BInit = {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::One)};
		TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_ResolveEditorSelection");
	}

// --------------------------------------------------------------------------------------

	TonemapPSO::TonemapPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Tonemap.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		TRefCountPtr<BlendState> BState = nullptr;
		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_Tonemap");
	}

// --------------------------------------------------------------------------------------

	AmbientOcclusionPSO::AmbientOcclusionPSO(D3D12CommandList* CommandList, CommonResources* CR)
	{
		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceAmbientOcclusionSetup.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = {};
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_R16G16B16A16_FLOAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_SetupPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_SetupPSO->PipelineState, "PSO_SetupAO");
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceAmbientOcclusion.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { L"USE_AO_SETUP_AS_INPUT=1" };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_R8_UNORM };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_HalfPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_HalfPSO->PipelineState, "HalfPSO_AO");
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceAmbientOcclusion.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = {};
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_R8_UNORM };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_MainPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_MainPSO->PipelineState, "PSO_AO");
		}
	}

// --------------------------------------------------------------------------------------

	ScreenSpaceReflectionPSO::ScreenSpaceReflectionPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ScreenSpaceReflection.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		TRefCountPtr<BlendState> BState = nullptr;
		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_COLOR_DEFERRED_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_ScreenSpaceReflection");
	}

// --------------------------------------------------------------------------------------

	ReflectionEnvironemntPSO::ReflectionEnvironemntPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\ReflectionEnvironment.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		BlendStateInitializer BInit = {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::One, EBlendFactor::One, EBlendOperation::Add, EBlendFactor::One, EBlendFactor::One)};
		TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_COLOR_DEFERRED_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_ReflectionEnvironemnt");
	}

// --------------------------------------------------------------------------------------

	TAAPSO::TAAPSO( D3D12CommandList* CommandList )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\TemporalAA.hlsl" ) );
		ID3DBlob* ComputeShaderBlob;
		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

		ComputeShader* CShader = new ComputeShader();
		CShader->ByteCode.pShaderBytecode = ComputeShaderBlob->GetBufferPointer();
		CShader->ByteCode.BytecodeLength = ComputeShaderBlob->GetBufferSize();

		m_PSO = ComputePipelineState::Create(CommandList->GetParentDevice(), CShader, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_TAA");
	}

// --------------------------------------------------------------------------------------

	SceneDownSamplePSO::SceneDownSamplePSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\SceneDownSample.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		TRefCountPtr<BlendState> BState = nullptr;
		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { SCENE_DOWN_SAMPLE_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_SceneDownSample");
	}

// --------------------------------------------------------------------------------------

	BloomPSO::BloomPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\GaussianBloom.hlsl" ) );

		const std::wstring StaticSampleMacro = std::wstring(L"STATIC_SAMPLE_COUNT=") + std::to_wstring(BLOOM_STATIC_SAMPLE_COUNT);

		{
			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { StaticSampleMacro.c_str(), L"BLOOM_Y=1" };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { BLOOM_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_BloomYPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_BloomYPSO->PipelineState, "PSO_BloomY");
		}

		{
			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { StaticSampleMacro.c_str() };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { BLOOM_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_BloomXPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_BloomXPSO->PipelineState, "PSO_BloomX");
		}

		{
			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;

			const std::vector<const wchar_t*> Macros = { StaticSampleMacro.c_str(), L"BLOOM_ADDTIVE=1" };
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { BLOOM_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_BloomXAddtivePSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_BloomXAddtivePSO->PipelineState, "PSO_BloomXAddtive");
		}
	}

// --------------------------------------------------------------------------------------

	PositionOnlyDepthPSO::PositionOnlyDepthPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\PositionOnlyDepthVertexShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;

		const std::vector<const wchar_t*> Macros = {};
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		{
			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_Pos, VShader, nullptr, nullptr, nullptr, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::None);
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				0, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_CullNonePSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_CullNonePSO->PipelineState, "PSO_PositionOnlyDepth_CullNone");
		}

		{
			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_Pos, VShader, nullptr, nullptr, nullptr, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				0, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_CullBackPSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_CullBackPSO->PipelineState, "PSO_PositionOnlyDepth_CullBack");
		}
	}

// --------------------------------------------------------------------------------------

	SpriteEditorPrimitivePSO::SpriteEditorPrimitivePSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {};
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Sprite.hlsl" ) );
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

		BlendStateInitializer BInit = {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::One, EBlendFactor::One)};
		TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_SpriteEditorPrimitive");
	}

// --------------------------------------------------------------------------------------

	SpriteHitProxyPSO::SpriteHitProxyPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Sprite.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = {L"HITPROXY_PASS=1"};

		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);


		TRefCountPtr<BlendState> BState = nullptr;
		TRefCountPtr<RasterizerState> RState = nullptr;

		DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_GUID_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_SpriteEditorPrimitive");
	}

// --------------------------------------------------------------------------------------

	LightPassPSO::LightPassPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\LightPassDeferred.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::wstring SideMacro = StringHelper::s2ws("SPOTLIGHT_STENCIL_SIDES=" + std::to_string(SPOTLIGHT_STENCIL_SIDES));
		std::wstring SliceMacro = StringHelper::s2ws("SPOTLIGHT_STENCIL_SLICES=" + std::to_string(SPOTLIGHT_STENCIL_SLICES));

		const std::vector<const wchar_t*> Macros = { SideMacro.c_str(), SliceMacro.c_str() };
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_Pos, VShader, nullptr, nullptr, PShader, nullptr);

		BlendStateInitializer BInit = {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::One, EBlendFactor::One, EBlendOperation::Add, EBlendFactor::One, EBlendFactor::One)};
		TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

		RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::Front);
		TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

		DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_COLOR_DEFERRED_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
			1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_LightpassDeferred");
	}

// --------------------------------------------------------------------------------------

	DebugLineThicknessPSO::DebugLineThicknessPSO( D3D12CommandList* CommandList, CommonResources* CR )
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\DebugLineThicknessShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;
		ID3DBlob* GeometeryShaderBlob;

		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);
		CompileShader( ShaderPath, L"Main_GS", L"gs_6_6", Macros, &GeometeryShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

		GeometryShader* GShader = new GeometryShader();
		GShader->ByteCode.pShaderBytecode = GeometeryShaderBlob->GetBufferPointer();
		GShader->ByteCode.BytecodeLength = GeometeryShaderBlob->GetBufferSize();


		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_LineColorThickness, VShader, nullptr, nullptr, PShader, GShader);

		TRefCountPtr<BlendState> BState = nullptr;

		RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::None);
		TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

		DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::LineList,
			1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_DebugLineThickness");
	}

// --------------------------------------------------------------------------------------

	DebugLinePSO::DebugLinePSO(D3D12CommandList* CommandList, CommonResources* CR)
	{
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\DebugLineShader.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		const std::vector<const wchar_t*> Macros;
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);

		VertexShader* VShader = new VertexShader();
		VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

		PixelShader* PShader = new PixelShader();
		PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

		BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_LineColorThickness, VShader, nullptr, nullptr, PShader, nullptr);

		TRefCountPtr<BlendState> BState = nullptr;

		RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::None);
		TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

		DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
		TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
		DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
		ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

		GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::LineList,
			1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

		m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
		SetName(m_PSO->PipelineState, "PSO_DebugLine");
	}

// --------------------------------------------------------------------------------------

#if WITH_EDITOR
	Texture2DToTextureCubePSO::Texture2DToTextureCubePSO( D3D12CommandList* CommandList, ID3D12RootSignature* RS, DXGI_FORMAT Format, CommonResources* CR)
	{
		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\Texture2DToTextureCube.hlsl" ) );

			ID3DBlob* VertexShaderBlob;
			ID3DBlob* PixelShaderBlob;
			ID3DBlob* GeometeryShaderBlob;

			const std::vector<const wchar_t*> Macros;
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);
			CompileShader( ShaderPath, L"Main_GS", L"gs_6_6", Macros, &GeometeryShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

			GeometryShader* GShader = new GeometryShader();
			GShader->ByteCode.pShaderBytecode = GeometeryShaderBlob->GetBufferPointer();
			GShader->ByteCode.BytecodeLength = GeometeryShaderBlob->GetBufferSize();

			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, GShader);

			TRefCountPtr<BlendState> BState = nullptr;
	
			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::None);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);

			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { Format };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, RS);
			SetName(m_PSO->PipelineState, "PSO_Texture2DToTextureCube");
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

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

			GeometryShader* GShader = new GeometryShader();
			GShader->ByteCode.pShaderBytecode = GeometeryShaderBlob->GetBufferPointer();
			GShader->ByteCode.BytecodeLength = GeometeryShaderBlob->GetBufferSize();

			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, GShader);

			TRefCountPtr<BlendState> BState = nullptr;
	
			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::None);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);

			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { Format };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			m_MipPSO = GraphicsPipelineState::Create( CommandList->GetParentDevice(), Init, RS );
			SetName(m_MipPSO->PipelineState, "PSO_Texture2DToTextureCubeMip");
		}
	}

// --------------------------------------------------------------------------------------

	BufferVisualizerPSO::BufferVisualizerPSO(D3D12CommandList* CommandList, CommonResources* CR)
	{
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
		std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\BufferVisualizer.hlsl" ) );

		ID3DBlob* VertexShaderBlob;
		ID3DBlob* PixelShaderBlob;

		std::vector<const wchar_t*> Macros = { L"BASECOLOR=1" };
		CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", Macros, &VertexShaderBlob);
		CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", Macros, &PixelShaderBlob);



		auto CreateVisualizerPSO = [&](std::vector<const wchar_t*> InMacros, TRefCountPtr<GraphicsPipelineState>& PSO, const std::string& Name)
		{
			CompileShader( ShaderPath, L"Main_VS", L"vs_6_6", InMacros, &VertexShaderBlob);
			CompileShader( ShaderPath, L"Main_PS", L"ps_6_6", InMacros, &PixelShaderBlob);

			VertexShader* VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = VertexShaderBlob->GetBufferSize();

			PixelShader* PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = PixelShaderBlob->GetBufferSize();

			BoundShaderStateInput BoundShaderState(CR->VertexDeclaration_PosUV, VShader, nullptr, nullptr, PShader, nullptr);

			TRefCountPtr<BlendState> BState = nullptr;
			TRefCountPtr<RasterizerState> RState = nullptr;

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);

			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PSO = GraphicsPipelineState::Create(CommandList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PSO->PipelineState, Name);
		};

		CreateVisualizerPSO( { L"BASECOLOR=1" }, m_BaseColorPSO, "PSO_BufferVisualizer_BaseColor" );
		CreateVisualizerPSO( { L"METALLIC=1" }, m_MetallicPSO, "PSO_BufferVisualizer_Metallic" );
		CreateVisualizerPSO({ L"ROUGHNESS=1" }, m_RoughnessPSO, "PSO_BufferVisualizer_Roughness");
		CreateVisualizerPSO({ L"MATERIAL_AO=1" }, m_MaterialAoPSO, "PSO_BufferVisualizer_MaterialAo");
		CreateVisualizerPSO({ L"SHADING_MODEL=1" }, m_ShadingModelPSO, "PSO_BufferVisualizer_ShadingModel");
		CreateVisualizerPSO({ L"WORLD_NORMAL=1" }, m_WorldNormalPSO, "PSO_BufferVisualizer_WorldNormal");
		CreateVisualizerPSO({ L"SUBSURFACE_COLOR=1" }, m_SubsurfaceColorPSO, "PSO_BufferVisualizer_SubsurfaceColor");
		CreateVisualizerPSO({ L"DEPTH=1" }, m_DepthPSO, "PSO_BufferVisualizer_Depth");
		CreateVisualizerPSO({ L"LINEAR_DEPTH=1" }, m_LinearDepthPSO, "PSO_BufferVisualizer_LinearDepth");
		CreateVisualizerPSO({ L"PRE_TONEMAP=1" }, m_PreTonemapPSO, "PSO_BufferVisualizer_Pretonemap");
		CreateVisualizerPSO({ L"SCREEN_SPACE_AO=1" }, m_ScreenSpaceAOPSO, "PSO_BufferVisualizer_SSAO");
		CreateVisualizerPSO({ L"BLOOM=1" }, m_Bloom, "PSO_BufferVisualizer_Bloom");
		CreateVisualizerPSO({ L"SCREEN_SPACE_REFLECTION=1" }, m_ScreenSpaceReflection, "PSO_BufferVisualizer_SSR");
	}

	TRefCountPtr<GraphicsPipelineState> BufferVisualizerPSO::GetPSOForBufferVisualizer( EBufferVisualization BufferVialization )
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

	HZBPSO::HZBPSO( D3D12CommandList* CommandList )
	{
		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=1"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			ComputeShader* CShader = new ComputeShader();
			CShader->ByteCode.pShaderBytecode = ComputeShaderBlob->GetBufferPointer();
			CShader->ByteCode.BytecodeLength = ComputeShaderBlob->GetBufferSize();

			m_1Mip_PSO = ComputePipelineState::Create(CommandList->GetParentDevice(), CShader, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_1Mip_PSO->PipelineState, "PSO_HZB_1Mip");
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=2"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			ComputeShader* CShader = new ComputeShader();
			CShader->ByteCode.pShaderBytecode = ComputeShaderBlob->GetBufferPointer();
			CShader->ByteCode.BytecodeLength = ComputeShaderBlob->GetBufferSize();

			m_2Mip_PSO = ComputePipelineState::Create(CommandList->GetParentDevice(), CShader, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_2Mip_PSO->PipelineState, "PSO_HZB_2Mip");
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=3"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			ComputeShader* CShader = new ComputeShader();
			CShader->ByteCode.pShaderBytecode = ComputeShaderBlob->GetBufferPointer();
			CShader->ByteCode.BytecodeLength = ComputeShaderBlob->GetBufferSize();

			m_3Mip_PSO = ComputePipelineState::Create(CommandList->GetParentDevice(), CShader, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_3Mip_PSO->PipelineState, "PSO_HZB_3Mip");
		}

		{
			std::wstring ShaderPath = StringHelper::s2ws( Path::ConvertProjectPath( "\\Engine\\Content\\Shader\\HZB.hlsl" ) );
			ID3DBlob* ComputeShaderBlob;
			const std::vector<const wchar_t*> Macros = {L"MIP_LEVEL_COUNT=4"};
			CompileShader( ShaderPath, L"Main_CS", L"cs_6_6", Macros, &ComputeShaderBlob);

			ComputeShader* CShader = new ComputeShader();
			CShader->ByteCode.pShaderBytecode = ComputeShaderBlob->GetBufferPointer();
			CShader->ByteCode.BytecodeLength = ComputeShaderBlob->GetBufferSize();

			m_4Mip_PSO = ComputePipelineState::Create(CommandList->GetParentDevice(), CShader, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(m_4Mip_PSO->PipelineState, "PSO_HZB_4Mip");
		}
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