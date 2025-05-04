#include "DrnPCH.h"
#include "AssetPreviewTexture2DGuiLayer.h"

#if WITH_EDITOR

#include <imgui.h>
#include "Editor/Editor.h"
#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	AssetPreviewTexture2DGuiLayer::AssetPreviewTexture2DGuiLayer( Texture2D* InOwningAsset )
	{
		ViewCpuHandle.ptr = 0;
		ViewGpuHandle.ptr = 0;

		m_OwningAsset = AssetHandle<Texture2D>( InOwningAsset->m_Path );
		m_OwningAsset.Load();
	}

	AssetPreviewTexture2DGuiLayer::~AssetPreviewTexture2DGuiLayer()
	{
		m_OwningAsset->GuiLayer = nullptr;
		ReleaseHandles();
	}

	void AssetPreviewTexture2DGuiLayer::AllocateHandles()
	{
		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Alloc( &ViewCpuHandle, &ViewGpuHandle );
	}

	void AssetPreviewTexture2DGuiLayer::ReleaseHandles()
	{
		if (!(ViewCpuHandle.ptr == 0 && ViewGpuHandle.ptr == 0))
		{
			ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Free(ViewCpuHandle, ViewGpuHandle);

			ViewCpuHandle.ptr = 0;
			ViewGpuHandle.ptr = 0;
		}
	}

	void AssetPreviewTexture2DGuiLayer::Draw( float DeltaTime )
	{
		std::string name = m_OwningAsset->m_Path;
		name = Path::ConvertShortPath(name);
		name = Path::RemoveFileExtension(name);

		if (!ImGui::Begin(name.c_str(), &m_Open))
		{
			ImGui::End();
			return;
		}

		DrawMenu();

		ImVec2 Size = ImGui::GetContentRegionAvail();
		float BorderSize = ImGui::GetStyle().FramePadding.x;

		ImVec2 SidePanelSize = ImVec2( Editor::Get()->SidePanelSize, 0.0f );
		ImVec2 ViewportSize = ImVec2( Size.x - (SidePanelSize.x + 2 * BorderSize), 0.0f );

		ImGui::SameLine();
		if ( ImGui::BeginChild( "Viewport", ViewportSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened ) )
		{
			DrawViewportPanel();
		}
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild( "Detail", SidePanelSize, ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened) )
		{
			DrawDetailsPanel();

			ImGui::EndChild();
		}


		ImGui::End();
	}

	void AssetPreviewTexture2DGuiLayer::DrawMenu()
	{
		
	}

	void AssetPreviewTexture2DGuiLayer::DrawViewportPanel()
	{
		if (m_OwningAsset.IsValid())
		{
			ReleaseHandles();
			m_OwningAsset->UploadResources(Renderer::Get()->GetCommandList().get());
			
			// for now reallocate resources
			ID3D12Device* pDevice = Renderer::Get()->GetDevice()->GetD3D12Device().Get();
			AllocateHandles();

			D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
			
			descSRV.Texture2D.MipLevels       = m_OwningAsset->GetMipLevels();
			descSRV.Texture2D.MostDetailedMip = 0;
			descSRV.Format                    = m_OwningAsset->m_Format;
			descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
			descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			
			pDevice->CreateShaderResourceView( m_OwningAsset->GetResource(), &descSRV, ViewCpuHandle );
			ImGui::Image( (ImTextureID)ViewGpuHandle.ptr, ImVec2( m_OwningAsset->GetSizeX(), m_OwningAsset->GetSizeY()) );
		}
	}

	void AssetPreviewTexture2DGuiLayer::DrawDetailsPanel()
	{
		ImGui::Text("aeraerare");
	}

}

#endif