#include "DrnPCH.h"
#include "MaterialShared.h"

#if WITH_EDITOR
#include "imgui.h"
#endif

namespace Drn
{

	std::vector<VertexFactoryType*> VertexFactoryType::GlobalFactories;
	VertexFactoryType* VertexFactoryType::StaticMesh = new VertexFactoryType("StaticMesh", &CommonResources::Get()->VertexDeclaration_StaticMesh, &CommonResources::Get()->VertexDeclaration_Pos);
	VertexFactoryType* VertexFactoryType::Decal = new VertexFactoryType("Decal", &CommonResources::Get()->VertexDeclaration_Pos, nullptr);

	void MaterialUniformParameters::Serialize( Archive& Ar )
	{
		if (Ar.IsLoading())
		{
			Clear();

			uint8 Texture2DCount;
			Ar >> Texture2DCount;
			m_Texture2DSlots.reserve(Texture2DCount);
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots.push_back({});
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount;
			Ar >> TextureCubeCount;
			m_TextureCubeSlots.reserve(TextureCubeCount);
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots.push_back({});
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount;
			Ar >> ScalarCount;
			m_FloatSlots.reserve(ScalarCount);
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots.push_back({});
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count;
			Ar >> Vector4Count;
			m_Vector4Slots.reserve(Vector4Count);
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots.push_back({});
				m_Vector4Slots[i].Serialize(Ar);
			}
		}

		else
		{
			uint8 Texture2DCount = m_Texture2DSlots.size();
			Ar << Texture2DCount;
			for (int i = 0; i < Texture2DCount; i++)
			{
				m_Texture2DSlots[i].Serialize(Ar);
			}

			uint8 TextureCubeCount = m_TextureCubeSlots.size();
			Ar << TextureCubeCount;
			for (int i = 0; i < TextureCubeCount; i++)
			{
				m_TextureCubeSlots[i].Serialize(Ar);
			}

			uint8 ScalarCount = m_FloatSlots.size();
			Ar << ScalarCount;
			for (int i = 0; i < ScalarCount; i++)
			{
				m_FloatSlots[i].Serialize(Ar);
			}

			uint8 Vector4Count = m_Vector4Slots.size();
			Ar << Vector4Count;
			for (int i = 0; i < Vector4Count; i++)
			{
				m_Vector4Slots[i].Serialize(Ar);
			}
		}
	}

	void MaterialUniformParameters::UploadResources( class D3D12CommandList* CommandList )
	{
		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			if ( m_Texture2DSlots[i].m_Texture2D.IsValid())
			{
				m_Texture2DSlots[i].m_Texture2D->UploadResources(CommandList);
			}
		}

		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			if (m_TextureCubeSlots[i].m_TextureCube.IsValid())
			{
				m_TextureCubeSlots[i].m_TextureCube->UploadResources(CommandList);
			}
		}

		const int32 Vector4SlotCount = m_Vector4Slots.size() * 4;
		const int32 ScalarSlotCount = m_FloatSlots.size();
		const int32 Texture2DSlotCount = m_Texture2DSlots.size() * 2;
		const int32 TextureCubeSlotCount = m_TextureCubeSlots.size() * 2;
		const int32 SlotCount = Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount + TextureCubeSlotCount;

		std::vector<uint32> Parameters;
		Parameters.resize(SlotCount);

		for (int i = 0; i < m_Vector4Slots.size(); i++)
		{
			*(Vector4*)(&Parameters[i * 4]) = m_Vector4Slots[i].m_Value;
		}

		for (int i = 0; i < m_FloatSlots.size(); i++)
		{
			*(float*)(&Parameters[i + Vector4SlotCount]) = m_FloatSlots[i].m_Value;
		}

		for (int i = 0; i < m_Texture2DSlots.size(); i++)
		{
			Parameters[i * 2 + Vector4SlotCount + ScalarSlotCount] = m_Texture2DSlots[i].m_Texture2D.IsValid() ? m_Texture2DSlots[i].m_Texture2D->GetTextureIndex() : 0;
			Parameters[i * 2 + 1 + Vector4SlotCount + ScalarSlotCount] = m_Texture2DSlots[i].m_Texture2D.IsValid() ? m_Texture2DSlots[i].m_Texture2D->GetSamplerIndex() : 0;
		}

		for (int i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			Parameters[i * 2 + Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount] = m_TextureCubeSlots[i].m_TextureCube.IsValid() ? m_TextureCubeSlots[i].m_TextureCube->GetTextureIndex() : 0;
			Parameters[i * 2 + 1 + Vector4SlotCount + ScalarSlotCount + Texture2DSlotCount] = m_TextureCubeSlots[i].m_TextureCube.IsValid() ? m_TextureCubeSlots[i].m_TextureCube->GetSamplerIndex() : 0;
		}

		if ( Parameters.size() > 0 )
		{
			//uint32 Size = Align(TextureIndices.size() * sizeof(uint32), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			uint32 Size = Parameters.size() * sizeof(uint32);
			ParametersBuffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), Size, EUniformBufferUsage::SingleFrame, Parameters.data());
		}
	}

	void MaterialUniformParameters::OverrideParams( MaterialUniformParameters& Source )
	{
		for (Texture2DProperty& Prop : m_Texture2DSlots)
		{
			for (Texture2DProperty& SourceProp : Source.m_Texture2DSlots)
			{
				if (Prop.m_Name == SourceProp.m_Name)
				{
					SourceProp.m_Texture2D.LoadChecked();
					if (SourceProp.m_Texture2D.IsValid())
					{
						Prop.m_Texture2D = SourceProp.m_Texture2D;
					}
				}
			}
		}

		for (TextureCubeProperty& Prop : m_TextureCubeSlots)
		{
			for (TextureCubeProperty& SourceProp : Source.m_TextureCubeSlots)
			{
				if (Prop.m_Name == SourceProp.m_Name)
				{
					SourceProp.m_TextureCube.LoadChecked();
					if (SourceProp.m_TextureCube.IsValid())
					{
						Prop.m_TextureCube = SourceProp.m_TextureCube;
					}
				}
			}
		}

		for (FloatProperty& Prop : m_FloatSlots)
		{
			for (FloatProperty& SourceProp : Source.m_FloatSlots)
			{
				if (Prop.m_Name == SourceProp.m_Name)
				{
					Prop.m_Value = SourceProp.m_Value;
				}
			}
		}

		for (Vector4Property& Prop : m_Vector4Slots)
		{
			for (Vector4Property& SourceProp : Source.m_Vector4Slots)
			{
				if (Prop.m_Name == SourceProp.m_Name)
				{
					Prop.m_Value = SourceProp.m_Value;
				}
			}
		}
	}

	void MaterialUniformParameters::SetNamedTexture2D( const std::string& Name, AssetHandle<Texture2D> TextureAsset )
	{
		for (uint8 i = 0; i < m_Texture2DSlots.size(); i++)
		{
			const Texture2DProperty& TextureSlot = m_Texture2DSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTexture2D(i, TextureAsset);
				return;
			}
		}
	}

	void MaterialUniformParameters::SetIndexedTexture2D( uint8 Index, AssetHandle<Texture2D> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_Texture2DSlots.size())
		{
			m_Texture2DSlots[Index].m_Texture2D = TextureAsset;
		}
	}

	void MaterialUniformParameters::SetNamedTextureCube( const std::string& Name, AssetHandle<TextureCube> TextureAsset )
	{
		for (uint8 i = 0; i < m_TextureCubeSlots.size(); i++)
		{
			const TextureCubeProperty& TextureSlot = m_TextureCubeSlots[i];

			if (TextureSlot.m_Name == Name)
			{
				SetIndexedTextureCube(i, TextureAsset);
				return;
			}
		}
	}

	void MaterialUniformParameters::SetIndexedTextureCube( uint8 Index, AssetHandle<TextureCube> TextureAsset )
	{
		if (TextureAsset.IsValid() && Index >= 0 && Index < m_TextureCubeSlots.size())
		{
			m_TextureCubeSlots[Index].m_TextureCube = TextureAsset;
		}
	}

	void MaterialUniformParameters::SetIndexedScalar( uint32 Index, float Value )
	{
		if (Index >= 0 && Index < m_FloatSlots.size())
		{
			m_FloatSlots[Index].m_Value = Value;
		}
	}

	void MaterialUniformParameters::SetIndexedVector( uint32 Index, const Vector4& Value )
	{
		if (Index >= 0 && Index < m_Vector4Slots.size())
		{
			m_Vector4Slots[Index].m_Value = Value;
		}
	}

	void MaterialUniformParameters::SetNamedScalar( const std::string& Name, float Value )
	{
		for ( int32 i = 0; i < m_FloatSlots.size(); i++)
		{
			if (m_FloatSlots[i].m_Name == Name)
			{
				SetIndexedScalar(i, Value);
			}
		}
	}

	void MaterialUniformParameters::SetNamedVector4( const std::string& Name, const Vector4& Value )
	{
		for ( int32 i = 0; i < m_Vector4Slots.size(); i++)
		{
			if (m_Vector4Slots[i].m_Name == Name)
			{
				SetIndexedVector(i, Value);
			}
		}
	}

// ---------------------------------------------------------------------------------------------------------

	BoundShaderStateInput GetShaderStateInput(VertexDeclaration* VDeclaration, ShaderBlob& Blob)
	{
		VertexShader* VShader = nullptr;
		HullShader* HShader = nullptr;
		DomainShader* DShader = nullptr;
		PixelShader* PShader = nullptr;
		GeometryShader* GShader = nullptr;

		if (Blob.m_VS)
		{
			VShader = new VertexShader();
			VShader->ByteCode.pShaderBytecode = Blob.m_VS->GetBufferPointer();
			VShader->ByteCode.BytecodeLength = Blob.m_VS->GetBufferSize();
		}

		if (Blob.m_HS)
		{
			HShader = new HullShader();
			HShader->ByteCode.pShaderBytecode = Blob.m_HS->GetBufferPointer();
			HShader->ByteCode.BytecodeLength = Blob.m_HS->GetBufferSize();
		}

		if (Blob.m_DS)
		{
			DShader = new DomainShader();
			DShader->ByteCode.pShaderBytecode = Blob.m_DS->GetBufferPointer();
			DShader->ByteCode.BytecodeLength = Blob.m_DS->GetBufferSize();
		}

		if (Blob.m_PS)
		{
			PShader = new PixelShader();
			PShader->ByteCode.pShaderBytecode = Blob.m_PS->GetBufferPointer();
			PShader->ByteCode.BytecodeLength = Blob.m_PS->GetBufferSize();
		}

		if (Blob.m_GS)
		{
			GShader = new GeometryShader();
			GShader->ByteCode.pShaderBytecode = Blob.m_GS->GetBufferPointer();
			GShader->ByteCode.BytecodeLength = Blob.m_GS->GetBufferSize();
		}

		return BoundShaderStateInput(VDeclaration, VShader, HShader, DShader, PShader, GShader);
	}

	void MaterialShader::UploadPipelineState(D3D12CommandList* CmdList, Material* InMaterial)
	{
		if (MaterialStage == EMaterialStage::Main)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_COLOR_DEFERRED_FORMAT, GBUFFER_BASE_COLOR_FORMAT, GBUFFER_WORLD_NORMAL_FORMAT, GBUFFER_MASKS_FORMAT, GBUFFER_MASKS_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				_countof(TargetFormats), TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_MainPass_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::Prepass)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				0, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_PrePass_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::PointLightShadow)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(true, ECompareFunction::LessEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
	
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				0, TargetFormats, TargetFlags, DXGI_FORMAT_D16_UNORM, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_PointLightShadowDepthPass_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::SpotLightShadow)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(true, ECompareFunction::LessEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
	
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				0, TargetFormats, TargetFlags, DXGI_FORMAT_D16_UNORM, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_SpotLightShadowDepthPass_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::StaticMeshDecal)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);

			BlendStateInitializer BInit( {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::InverseSourceAlpha)} );
			BInit.bUseIndependentRenderTargetBlendStates = false;
			TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DECAL_BASE_COLOR_FORMAT, DECAL_NORMAL_FORMAT, DECAL_MASKS_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				3, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_StaticMeshDecalPass_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::Decal)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);

			BlendStateInitializer BInit( {BlendStateInitializer::RenderTarget(EBlendOperation::Add, EBlendFactor::SourceAlpha, EBlendFactor::InverseSourceAlpha, EBlendOperation::Add, EBlendFactor::Zero, EBlendFactor::InverseSourceAlpha)} );
			BInit.bUseIndependentRenderTargetBlendStates = false;
			TRefCountPtr<BlendState> BState = BlendState::Create(BInit);

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, ERasterizerCullMode::Front);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(false, ECompareFunction::Always);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DECAL_BASE_COLOR_FORMAT, DECAL_NORMAL_FORMAT, DECAL_MASKS_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				3, TargetFormats, TargetFlags, DXGI_FORMAT_UNKNOWN, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_DecalPass_" + InMaterial->GetMaterialName());
		}

#if WITH_EDITOR
		else if (MaterialStage == EMaterialStage::Hitproxy)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { GBUFFER_GUID_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_HitProxyPass_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::EditorPrimitive)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DISPLAY_OUTPUT_FORMAT };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				1, TargetFormats, TargetFlags, DEPTH_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_EditorPrimitive_" + InMaterial->GetMaterialName());
		}

		else if (MaterialStage == EMaterialStage::EditorSelection)
		{
			BoundShaderStateInput BoundShaderState = GetShaderStateInput(VertexFactory->GetVertexDeclaration(), Blob);
			BoundShaderState.m_PixelShader = nullptr;

			TRefCountPtr<BlendState> BState = nullptr;

			RasterizerStateInitializer RInit(ERasterizerFillMode::Solid, InMaterial->IsTwoSided() ? ERasterizerCullMode::None : ERasterizerCullMode::Back);
			TRefCountPtr<RasterizerState> RState = RasterizerState::Create(RInit);

			DepthStencilStateInitializer DInit(true, ECompareFunction::GreaterEqual,
				true, ECompareFunction::Always, EStencilOp::Replace, EStencilOp::Replace, EStencilOp::Replace,
				true, ECompareFunction::Always, EStencilOp::Replace, EStencilOp::Replace, EStencilOp::Replace);
			TRefCountPtr<DepthStencilState> DState = DepthStencilState::Create(DInit);
		
			DXGI_FORMAT TargetFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { DXGI_FORMAT_UNKNOWN };
			ETextureCreateFlags TargetFlags[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = { ETextureCreateFlags::None };

			GraphicsPipelineStateInitializer Init(BoundShaderState, BState, RState, DState, EPrimitiveType::TriangleList,
				0, TargetFormats, TargetFlags, DEPTH_STENCIL_FORMAT, ETextureCreateFlags::None, EDepthStencilViewType::DepthWrite, 1);

			PipelineState = GraphicsPipelineState::Create(CmdList->GetParentDevice(), Init, Renderer::Get()->m_BindlessRootSinature.Get());
			SetName(PipelineState->PipelineState, "PSO_SelectionPass_" + InMaterial->GetMaterialName());
		}
#endif
	}

	void MaterialShader::Bind( D3D12CommandList* CmdList )
	{
		CmdList->SetGraphicPipelineState(PipelineState);
	}

	void MaterialShaders::UploadPipelineStates( D3D12CommandList* CmdList, Material* InMaterial )
	{
		for (MaterialShader& MatShader : Shaders)
		{
			MatShader.UploadPipelineState(CmdList, InMaterial);
		}
	}

	const wchar_t* GetVertexFactoryShaderMacro( VertexFactoryType* VertexFactory )
	{
		if (VertexFactory == VertexFactoryType::StaticMesh)
		{
			return L"STATICMESH=1";
		}

		else if (VertexFactory == VertexFactoryType::Decal)
		{
			return L"DECAL=1";
		}

		drn_check(false);
		return L"STATICMESH=1";
	}

#if WITH_EDITOR
	void MaterialShaderParameters::Draw()
	{
		bool bMasked = bIsMasked; ImGui::Checkbox("Masked", &bMasked);
		bool bTwoSided = bIsTwoSided; ImGui::Checkbox("TwoSided", &bTwoSided);

		bool bInstacedStaticMesh = bIsUsedWithInstancedStaticMesh; ImGui::Checkbox("Instanced Static Mesh", &bInstacedStaticMesh);
	}
#endif

}  // namespace Drn