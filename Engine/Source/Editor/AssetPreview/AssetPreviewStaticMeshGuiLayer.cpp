#include "DrnPCH.h"
#include "AssetPreviewStaticMeshGuiLayer.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "Editor/Editor.h"
#include "Editor/FileImportMenu/FileImportMenu.h"

LOG_DEFINE_CATEGORY( LogStaticMeshPreview, "StaticMeshPreview" );

namespace Drn
{
	AssetPreviewStaticMeshGuiLayer::AssetPreviewStaticMeshGuiLayer(StaticMesh* InOwningAsset)
	{
		LOG(LogStaticMeshPreview, Info, "opening %s", InOwningAsset->m_Path.c_str());

		// make reference to prevent destruction when this gui is open
		m_OwningAsset = AssetHandle<StaticMesh>(InOwningAsset->m_Path);
		m_OwningAsset.Load();

		PreviewMesh = new StaticMeshComponent();
		PreviewMesh->SetMesh(m_OwningAsset);

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
		LOG(LogStaticMeshPreview, Info, "closing %s", m_OwningAsset->m_Path.c_str());

		if (PreviewScene)
		{
			if (MainView)
			{
				PreviewScene->RemoveAndInvalidateSceneRenderer(MainView);
			}

			Renderer::Get()->RemoveAndInvalidateScene(PreviewScene);
		}

		if (PreviewWorld)
		{
			delete PreviewWorld;
			PreviewWorld = nullptr;
		}

		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Free(ViewCpuHandle, ViewGpuHandle);

		m_OwningAsset->GuiLayer = nullptr;
	}

	void AssetPreviewStaticMeshGuiLayer::Draw()
	{
		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			MainView->SetRenderingEnabled(false);

			ImGui::End();
			return;
		}

		MainView->SetRenderingEnabled(true);

		DrawMenu();
		DrawViewport();
		DrawSidePanel();

		ImGui::End();
	}

	void AssetPreviewStaticMeshGuiLayer::DrawMenu()
	{
		if ( ImGui::BeginMainMenuBar() )
		{
			if ( ImGui::BeginMenu( "File" ) )
			{
				ImGui::MenuItem( "nothing" );
				ImGui::EndMenu();
			}

			if ( ImGui::BeginMenu( "Asset Manager" ) )
			{
				if ( ImGui::MenuItem( "log live assets" ) )
				{
					AssetManager::Get()->ReportLiveAssets();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void AssetPreviewStaticMeshGuiLayer::DrawSidePanel()
	{
		ImGui::Begin("Details");

		if (ImGui::Button( "save" ))
		{
			m_OwningAsset.Get()->Save();
		}

		ImGui::Separator();

		ImGui::Text("source file: %s", m_OwningAsset.Get()->m_SourcePath != NAME_NULL ? m_OwningAsset.Get()->m_SourcePath.c_str() : "...");
		
		if (ImGui::Button("reimport"))
		{
			m_OwningAsset.Get()->Import();
		}

		if ( ImGui::Button( "select" ) )
		{
			ShowSourceFileSelection();
		}

		ImGui::Separator();

		ImGui::InputFloat( "ImportScale", &m_OwningAsset.Get()->ImportScale);
		
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

	void AssetPreviewStaticMeshGuiLayer::ShowSourceFileSelection()
	{
		Editor::Get()->OpenImportMenu(
			"Select source file", FileImportMenu::FileFilter_Any(),
			std::bind( &AssetPreviewStaticMeshGuiLayer::OnSelectedSourceFile, this, std::placeholders::_1 ) );
	}

	void AssetPreviewStaticMeshGuiLayer::OnSelectedSourceFile( std::string FilePath )
	{
		m_OwningAsset->m_SourcePath = FilePath;
		m_OwningAsset->Import();
	}

	void AssetPreviewStaticMeshGuiLayer::SetCurrentFocus()
	{
		
	}

}

#endif