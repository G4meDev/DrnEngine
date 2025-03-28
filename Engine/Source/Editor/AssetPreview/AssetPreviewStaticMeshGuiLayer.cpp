#include "DrnPCH.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "AssetPreviewStaticMesh.h"

namespace Drn
{
	AssetPreviewStaticMeshGuiLayer::AssetPreviewStaticMeshGuiLayer(AssetPreviewStaticMesh* InOwningAsset)
		: m_OwningAsset(InOwningAsset)
	{
		PreviewMesh = new StaticMeshComponent();
		PreviewMesh->SetMesh(Renderer::Get()->CubeStaticMeshAsset);

		// TODO: lifetime management
		PreviewWorld = new World();
		PreviewWorld->AddStaticMeshCompponent(PreviewMesh);

		PreviewScene = Renderer::Get()->AllocateScene(PreviewWorld);
		MainView = PreviewScene->AllocateSceneRenderer();

// -------------------------------------------------------------------------------------------------------------------------

		ID3D12Device* pDevice = Renderer::Get()->GetDevice()->GetD3D12Device().Get();

		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Alloc( &ViewCpuHandle, &ViewGpuHandle );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		pDevice->CreateShaderResourceView( MainView->GetViewResource(), &descSRV, ViewCpuHandle );
	}

	AssetPreviewStaticMeshGuiLayer::~AssetPreviewStaticMeshGuiLayer()
	{
		
	}

	void AssetPreviewStaticMeshGuiLayer::Draw()
	{
		if (!ImGui::Begin(m_OwningAsset->GetPath().c_str()))
		{
			MainView->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}
	
		MainView->SetRenderingEnabled(true);

		DrawViewport();

		DrawSidePanel();

		ImGui::ShowDemoWindow();

		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawSidePanel()
	{
		ImGui::Begin("Details");

		if (ImGui::Button( "save" ))
		{
			m_OwningAsset->Save();
		}

		ImGui::Separator();

		ImGui::Text("source file: %s", m_OwningAsset->GetSourcePath() != NAME_NULL ? m_OwningAsset->GetSourcePath().c_str() : "...");
		
		if (ImGui::Button("reimport"))
		{
			m_OwningAsset->Reimport();
		}

		ImGui::Button("select");

		ImGui::Separator();

		ImGui::InputFloat( "ImportScale", &m_OwningAsset->ImportScale);
		
		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawViewport()
	{
		const ImVec2 AvaliableSize = ImGui::GetContentRegionAvail();
		IntPoint ImageSize = IntPoint( (int)AvaliableSize.x, (int)AvaliableSize.y );

		ImageSize.X = std::max( ImageSize.X, 1 );
		ImageSize.Y = std::max( ImageSize.Y, 1 );

		if ( CachedViewportSize != ImageSize )
		{
				CachedViewportSize = ImageSize;
				OnViewportSizeChanged( CachedViewportSize );
		}

		ImGui::Image( (ImTextureID)ViewGpuHandle.ptr, ImVec2( CachedViewportSize.X, CachedViewportSize.Y ) );
	}

	void AssetPreviewStaticMeshGuiLayer::OnViewportSizeChanged( const IntPoint& NewSize )
	{
		MainView->ResizeView( NewSize );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Renderer::Get()->GetDevice()->GetD3D12Device()->CreateShaderResourceView(
			MainView->GetViewResource(), &descSRV, ViewCpuHandle );
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{
		
	}

}

#endif