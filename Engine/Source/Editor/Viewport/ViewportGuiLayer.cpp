#include "DrnPCH.h"
#include "ViewportGuiLayer.h"

#if WITH_EDITOR

#include "imgui.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

#include "Editor/Editor.h"

namespace Drn
{
	ViewportGuiLayer::ViewportGuiLayer()
	{
		ID3D12Device* pDevice = Renderer::Get()->GetDevice()->GetD3D12Device().Get();

		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Alloc( &ViewCpuHandle, &ViewGpuHandle );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		
		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		pDevice->CreateShaderResourceView( Renderer::Get()->GetViewportResource(), &descSRV, ViewCpuHandle );
	}

	ViewportGuiLayer::~ViewportGuiLayer() 
	{
		
	}

	void ViewportGuiLayer::Draw()
	{
		if (!ImGui::Begin( "Viewport" ))
		{
			ImGui::End();
			return;
		}

		ShowMenu();

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
		ImGui::End();

		ImGui::ShowDemoWindow();
	}

	void ViewportGuiLayer::ShowMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Asset Manager"))
			{
				if (ImGui::MenuItem("log live assets"))
				{
					AssetManager::Get()->ReportLiveAssets();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void ViewportGuiLayer::OnViewportSizeChanged( const IntPoint& NewSize )
	{
		Renderer::Get()->ViewportResized( CachedSize.X, CachedSize.Y );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Renderer::Get()->GetDevice()->GetD3D12Device()->CreateShaderResourceView( Renderer::Get()->GetViewportResource(), &descSRV, ViewCpuHandle );
	}

}
#endif