#include "DrnPCH.h"
#include "ViewportPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"

namespace Drn
{
	ViewportPanel::ViewportPanel()
	{
		ID3D12Device* pDevice = Renderer::Get()->GetDevice()->GetD3D12Device().Get();

		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Alloc( &ViewCpuHandle, &ViewGpuHandle );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		
		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		pDevice->CreateShaderResourceView( Renderer::Get()->MainSceneRenderer->GetViewResource(), &descSRV, ViewCpuHandle );
	}

	ViewportPanel::~ViewportPanel()
	{
		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Free(ViewCpuHandle, ViewGpuHandle);
	}

	void ViewportPanel::Draw( float DeltaTime )
	{
		if (CameraInputHandler.Tick(DeltaTime))
		{
			CameraActor* Cam = Renderer::Get()->m_CameraActor;
			Cam->ApplyViewportInput(CameraInputHandler, CameraMovementSpeed, CameraRotationSpeed);
		}

		const ImVec2 AvaliableSize = ImGui::GetContentRegionAvail();
		IntPoint     ImageSize     = IntPoint( (int)AvaliableSize.x, (int)AvaliableSize.y );

		ImageSize.X = std::max( ImageSize.X, 1 );
		ImageSize.Y = std::max( ImageSize.Y, 1 );

		if ( CachedSize != ImageSize )
		{
			CachedSize = ImageSize;
			OnViewportSizeChanged(CachedSize);
		}

		ImGui::Image( (ImTextureID)ViewGpuHandle.ptr, ImVec2( CachedSize.X, CachedSize.Y) );
	}

	void ViewportPanel::OnViewportSizeChanged( const IntPoint& NewSize )
	{
		Renderer::Get()->MainSceneRenderer->ResizeView( NewSize );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Renderer::Get()->GetDevice()->GetD3D12Device()->CreateShaderResourceView( Renderer::Get()->MainSceneRenderer->GetViewResource(), &descSRV, ViewCpuHandle );
	}

}

#endif