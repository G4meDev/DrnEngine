#include "DrnPCH.h"
#include "ReflectionEnvironmentBuffer.h"

#include "Runtime/Renderer/RenderBuffer/GBuffer.h"
#include "Runtime/Renderer/RenderBuffer/ScreenSpaceReflectionBuffer.h"
#include "Runtime/Renderer/RenderBuffer/RenderBufferAO.h"

#include "Runtime/Engine/SkyLightSceneProxy.h"
#include "Runtime/Engine/ReflectionCaptureProxy.h"

namespace Drn
{
	ReflectionEnvironmentBuffer::ReflectionEnvironmentBuffer()
	{}

	ReflectionEnvironmentBuffer::~ReflectionEnvironmentBuffer()
	{}

	void ReflectionEnvironmentBuffer::Init()
	{
		RenderBuffer::Init();
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();

	}

	void ReflectionEnvironmentBuffer::Resize( const IntPoint& Size )
	{
		RenderBuffer::Resize(Size);
		ID3D12Device* Device = Renderer::Get()->GetD3D12Device();
	}

	void ReflectionEnvironmentBuffer::Clear( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void ReflectionEnvironmentBuffer::Bind( ID3D12GraphicsCommandList2* CommandList )
	{
		
	}

	void ReflectionEnvironmentBuffer::GenerateSkycubemap(class D3D12CommandList* CommandList, SceneRenderer* Renderer)
	{
		SkyLightSceneProxy* SkyProxy = Renderer->GetScene()->m_SkyLightProxies.size() > 0 ? *Renderer->GetScene()->m_SkyLightProxies.begin() : nullptr;
		AssetHandle<TextureCube> SkyCubemap = SkyProxy ? SkyProxy->GetCubemap() : AssetHandle<TextureCube>();

		if (GeneratedCubemap.IsValid() && !SkyCubemap.IsValid())
		{
			GeneratedCubemap = nullptr;
			GeneratedCubemapIradiance = nullptr;
			LastUsedCubemap = NAME_NULL;
			return;
		}

		else if (SkyCubemap.IsValid() && SkyCubemap.GetPath() != LastUsedCubemap)
		{
			//Renderer::Get()->MarkFrameForCapture();

			LastUsedCubemap = SkyCubemap.GetPath();
			
			const int32 BaseResolution = 128;
			const int32 NumMips = std::log2(BaseResolution) + 1;
			
			RenderResourceCreateInfo RawTextureCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "SkyCubemap_Raw" );
			TRefCountPtr<RenderTextureCube> RawCubemap = RenderTextureCube::Create(CommandList, BaseResolution, DXGI_FORMAT_R16G16B16A16_FLOAT, NumMips, 1, false,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::UAV | ETextureCreateFlags::NoFastClear), RawTextureCreateInfo);

			RenderResourceCreateInfo TextureCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "SkyCubemap" );
			GeneratedCubemap = RenderTextureCube::Create(CommandList, BaseResolution, DXGI_FORMAT_R16G16B16A16_FLOAT, NumMips, 1, false,
				(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::UAV | ETextureCreateFlags::NoFastClear), TextureCreateInfo);

			// copy first mip from source texture
			{
				for (int32 i = 0; i < 6; i++)
				{
					uint32 SubresourceIndex = D3D12CalcSubresource(0, i, 0, NumMips, 6);
					CommandList->AddTransitionBarrier(RawCubemap->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, SubresourceIndex);
				}

				TRefCountPtr<ShaderResourceView> SourceSrv = ShaderResourceView::CreateForMipLevel(SkyCubemap->GetRenderTexture(), 0);
			
				D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
				Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				Desc.Texture2DArray.ArraySize = 6;
				Desc.Texture2DArray.MipSlice = 0;
				Desc.Texture2DArray.FirstArraySlice = 0;
				TRefCountPtr<UnorderedAccessView> DestUav = new UnorderedAccessView(CommandList->GetParentDevice(), Desc, RawCubemap->m_ResourceLocation);
			
				int32 ThreadGroupSize = 8;
				int32 NumGroups = Math::DivideAndRoundUp(BaseResolution, ThreadGroupSize);
				int32 FaceGroupSize = NumGroups * ThreadGroupSize;
			
				IntPoint ValidDisppatchCoord(BaseResolution, BaseResolution);
			
				CommandList->SetComputePipelineState(CommonResources::Get()->m_ResizeSkycubemapPSO->m_PSO);
				CommandList->SetComputeRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetComputeRootConstant( Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2 );
				//CommandList->SetComputeRootConstant(NumMips, 3);
				//CommandList->SetComputeRootConstant(Mips_Index, 4);
				CommandList->SetComputeRootConstant(FaceGroupSize, 5);
				CommandList->SetComputeRootConstants(2, &ValidDisppatchCoord, 6);
				CommandList->SetComputeRootConstant(SourceSrv->GetDescriptorHeapIndex(), 8);
				CommandList->SetComputeRootConstant(DestUav->GetDescriptorHeapIndex(), 9);

				CommandList->DispatchComputeShader(NumGroups * 6, NumGroups, 1);

				if (true)
				{
					Vector4 LowerHemisphereColor(0, 0, 0, 1);

					CommandList->SetComputePipelineState(CommonResources::Get()->m_ApplyLowerHemisphereColorPSO->m_PSO);
					CommandList->SetComputeRootConstants(4, &LowerHemisphereColor, 12);

					CommandList->DispatchComputeShader(NumGroups * 6, NumGroups, 1);
				}
			
				for (int32 i = 0; i < 6; i++)
				{
					uint32 SubresourceIndex = D3D12CalcSubresource(0, i, 0, NumMips, 6);
					CommandList->AddTransitionBarrier(RawCubemap->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, SubresourceIndex);
				}
			}

			// generate mip chain
			{
				for (int32 MipIndex = 1; MipIndex < NumMips; MipIndex++)
				{
					for (int32 i = 0; i < 6; i++)
					{
						uint32 SubresourceIndex = D3D12CalcSubresource(MipIndex, i, 0, NumMips, 6);
						CommandList->AddTransitionBarrier(RawCubemap->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, SubresourceIndex);
					}

					const int32 MipResolution = 1 << (NumMips - MipIndex - 1);
			
					TRefCountPtr<ShaderResourceView> SourceSrv = ShaderResourceView::CreateForMipLevel(RawCubemap, MipIndex - 1);
			
					D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
					Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					Desc.Texture2DArray.ArraySize = 6;
					Desc.Texture2DArray.MipSlice = MipIndex;
					Desc.Texture2DArray.FirstArraySlice = 0;
					TRefCountPtr<UnorderedAccessView> DestUav = new UnorderedAccessView(CommandList->GetParentDevice(), Desc, RawCubemap->m_ResourceLocation);
			
					int32 ThreadGroupSize = 8;
					int32 NumGroups = Math::DivideAndRoundUp(MipResolution, ThreadGroupSize);
					int32 FaceGroupSize = NumGroups * ThreadGroupSize;
			
					IntPoint ValidDisppatchCoord(MipResolution, MipResolution);

					CommandList->SetComputePipelineState(CommonResources::Get()->m_CubemapDownsamplePSO->m_PSO);
					CommandList->SetComputeRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetComputeRootConstant( Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2 );
					CommandList->SetComputeRootConstant(NumMips, 3);
					CommandList->SetComputeRootConstant(MipIndex, 4);
					CommandList->SetComputeRootConstant(FaceGroupSize, 5);
					CommandList->SetComputeRootConstants(2, &ValidDisppatchCoord, 6);
					CommandList->SetComputeRootConstant(SourceSrv->GetDescriptorHeapIndex(), 8);
					CommandList->SetComputeRootConstant(DestUav->GetDescriptorHeapIndex(), 9);
			
					CommandList->DispatchComputeShader(NumGroups * 6, NumGroups, 1);
			
					for (int32 i = 0; i < 6; i++)
					{
						uint32 SubresourceIndex = D3D12CalcSubresource(MipIndex, i, 0, NumMips, 6);
						CommandList->AddTransitionBarrier(RawCubemap->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, SubresourceIndex);
					}
				}
			}

			// convolution
			{
				CommandList->AddTransitionBarrier(GeneratedCubemap->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

				for (int32 MipIndex = 0; MipIndex < NumMips; MipIndex++)
				{
				
					const int32 MipResolution = 1 << (NumMips - MipIndex - 1);
				
					D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
					Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					Desc.Texture2DArray.ArraySize = 6;
					Desc.Texture2DArray.MipSlice = MipIndex;
					Desc.Texture2DArray.FirstArraySlice = 0;
					TRefCountPtr<UnorderedAccessView> DestUav = new UnorderedAccessView(CommandList->GetParentDevice(), Desc, GeneratedCubemap->m_ResourceLocation);

					int32 ThreadGroupSize = 8;
					int32 NumGroups = Math::DivideAndRoundUp(MipResolution, ThreadGroupSize);
					int32 FaceGroupSize = NumGroups * ThreadGroupSize;
				
					IntPoint ValidDisppatchCoord(MipResolution, MipResolution);
				
					CommandList->SetComputePipelineState(CommonResources::Get()->m_ConvolveSpecularPSO->m_PSO);
					CommandList->SetComputeRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
					CommandList->SetComputeRootConstant( Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2 );
					CommandList->SetComputeRootConstant(NumMips, 3);
					CommandList->SetComputeRootConstant(MipIndex, 4);
					CommandList->SetComputeRootConstant(FaceGroupSize, 5);
					CommandList->SetComputeRootConstants(2, &ValidDisppatchCoord, 6);
					CommandList->SetComputeRootConstant(RawCubemap->GetShaderResourceView()->GetDescriptorHeapIndex(), 8);
					CommandList->SetComputeRootConstant(DestUav->GetDescriptorHeapIndex(), 9);
				
					CommandList->DispatchComputeShader(NumGroups * 6, NumGroups, 1);
				}

				CommandList->AddTransitionBarrier(GeneratedCubemap->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}

			// iradiance
			{
				const int32 IradianceTextureSize = 32;
				RenderResourceCreateInfo IradianceTextureCreateInfo( nullptr, nullptr, ClearValueBinding::Black, "SkyCubemapIradiance" );
				GeneratedCubemapIradiance = RenderTextureCube::Create(CommandList, IradianceTextureSize, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 1, false,
					(ETextureCreateFlags)(ETextureCreateFlags::ShaderResource | ETextureCreateFlags::UAV | ETextureCreateFlags::NoFastClear), TextureCreateInfo);

				CommandList->AddTransitionBarrier(GeneratedCubemapIradiance->GetResource(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

				TRefCountPtr<ShaderResourceView> SourceSrv = ShaderResourceView::CreateForMipLevel(RawCubemap, 0);

				D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
				Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				Desc.Texture2DArray.ArraySize = 6;
				Desc.Texture2DArray.MipSlice = 0;
				Desc.Texture2DArray.FirstArraySlice = 0;
				TRefCountPtr<UnorderedAccessView> DestUav = new UnorderedAccessView(CommandList->GetParentDevice(), Desc, GeneratedCubemapIradiance->m_ResourceLocation);

				int32 ThreadGroupSize = 8;
				int32 NumGroups = Math::DivideAndRoundUp(IradianceTextureSize, ThreadGroupSize);
				int32 FaceGroupSize = NumGroups * ThreadGroupSize;
				IntPoint ValidDisppatchCoord(IradianceTextureSize, IradianceTextureSize);

				const float SampleCount = 1024 * 16;
				const float UniformSampleSolidAngle = 2.0f * Math::PI / SampleCount;

				CommandList->SetComputePipelineState(CommonResources::Get()->m_ConvolveSpecularPSO->m_PSO);
				CommandList->SetComputeRootConstant(Renderer->ViewBuffer->GetViewIndex(), 0);
				CommandList->SetComputeRootConstant(UniformSampleSolidAngle, 1);
				CommandList->SetComputeRootConstant(Renderer::Get()->StaticSamplersBuffer->GetViewIndex(), 2);
				//CommandList->SetComputeRootConstant(NumMips, 3);
				//CommandList->SetComputeRootConstant(MipIndex, 4);
				CommandList->SetComputeRootConstant(FaceGroupSize, 5);
				CommandList->SetComputeRootConstants(2, &ValidDisppatchCoord, 6);
				CommandList->SetComputeRootConstant(SourceSrv->GetDescriptorHeapIndex(), 8);
				CommandList->SetComputeRootConstant(DestUav->GetDescriptorHeapIndex(), 9);
				
				CommandList->DispatchComputeShader(NumGroups * 6, NumGroups, 1);

				CommandList->AddTransitionBarrier(GeneratedCubemapIradiance->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			}
		}
	}

	void ReflectionEnvironmentBuffer::MapBuffer( D3D12CommandList* CommandList, SceneRenderer* Renderer )
	{
		uint32 ReflectionCaptureIndex = 0;
		for (ReflectionCaptureProxy* Proxy : Renderer->GetScene()->GetReflectionCaptureProxies())
		{
			drn_check(ReflectionCaptureIndex < MAX_REFLECTION_CAPTURE_COUNT);
			if (Proxy->CubemapIndex != 0)
			{
				m_Data.CaptureData[ReflectionCaptureIndex].ReflectionTexture = Proxy->CubemapIndex;
				m_Data.CaptureData[ReflectionCaptureIndex].PositionRadius = Vector4(Proxy->Position, Proxy->InfluenceRadius);
				m_Data.CaptureData[ReflectionCaptureIndex].OffsetBrightness = Vector4(Proxy->CaptureOffset, Proxy->Brightness);

				ReflectionCaptureIndex++;
			}
		}

		m_Data.NumReflectionCaptures = ReflectionCaptureIndex;

		m_Data.BaseColorTexture = Renderer->m_GBuffer->m_BaseColorTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.WorldNormalTexture = Renderer->m_GBuffer->m_WorldNormalTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.MasksTexture = Renderer->m_GBuffer->m_MasksTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.DepthTexture = Renderer->m_GBuffer->m_DepthTarget->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.SSRTexture = Renderer->m_ScreenSpaceReflectionBuffer->m_Target->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.PreintegratedGFTexture = CommonResources::Get()->m_PreintegratedGF->GetShaderResourceView()->GetDescriptorHeapIndex();
		m_Data.AOTexture = Renderer->m_AOBuffer->m_AOTarget->GetShaderResourceView()->GetDescriptorHeapIndex();

		SkyLightSceneProxy* SkyProxy = Renderer->GetScene()->m_SkyLightProxies.size() > 0 ? *Renderer->GetScene()->m_SkyLightProxies.begin() : nullptr;
		AssetHandle<TextureCube> SkyCubemap = SkyProxy ? SkyProxy->GetCubemap() : AssetHandle<TextureCube>();
		m_Data.SkyCubemapTexture = GeneratedCubemap.IsValid() ? GeneratedCubemap->GetShaderResourceView()->GetDescriptorHeapIndex() : 0;
		m_Data.SkyLightMipCount = GeneratedCubemap.IsValid() ? GeneratedCubemap->GetNumMips(): 0;
		m_Data.SkyLightColor = SkyProxy ? SkyProxy->GetColor() : Vector::ZeroVector;

		m_Data.SkyIradianceCubemapTexture = GeneratedCubemapIradiance.IsValid() ? GeneratedCubemapIradiance->GetShaderResourceView()->GetDescriptorHeapIndex() : 0;

		Buffer = RenderUniformBuffer::Create(CommandList->GetParentDevice(), sizeof(ReflectionEnvironmentData), EUniformBufferUsage::SingleFrame, &m_Data);
	}

}