#include "DrnPCH.h"
#include "ViewportPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"

#include <ImGuizmo.h>

LOG_DEFINE_CATEGORY( LogViewportPanel, "ViewportPanel" );

namespace Drn
{
	ViewportPanel::ViewportPanel(Scene* InScene)
	{
		Mat = XMMatrixIdentity();

		m_World = InScene->GetWorld();
		m_Scene = InScene;
		m_SceneRenderer = m_Scene->AllocateSceneRenderer();

		m_ViewportCamera = m_World->SpawnActor<CameraActor>();
		m_ViewportCamera->SetActorLocation(XMVectorSet(0, 0, 10, 0));
		m_ViewportCamera->SetActorRotation( Quat(0.0f, 0.0f, Math::PI) );

		m_ViewportCamera->SetActorLabel( "ViewportCamera" );
		m_ViewportCamera->SetTransient(true);

		m_SceneRenderer->m_CameraActor = m_ViewportCamera;

		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Alloc( &ViewCpuHandle, &ViewGpuHandle );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		
		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		
		Renderer::Get()->GetD3D12Device()->CreateShaderResourceView( m_SceneRenderer->GetViewResource(), &descSRV, ViewCpuHandle );
	}

	ViewportPanel::~ViewportPanel()
	{
		ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Free(ViewCpuHandle, ViewGpuHandle);
	}

	void ViewportPanel::Draw( float DeltaTime )
	{
		SCOPE_STAT(ViewportPanelDraw);

		if (CameraInputHandler.Tick(DeltaTime))
		{
			m_ViewportCamera->ApplyViewportInput(CameraInputHandler, CameraMovementSpeed, CameraRotationSpeed);
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

		if ( ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left) )
		{
			const ImVec2 RectMin = ImGui::GetItemRectMin();
			const ImVec2 MousePos = ImGui::GetMousePos();
			const IntPoint ClickPos = IntPoint( MousePos.x - RectMin.x, MousePos.y - RectMin.y );
			Guid SelectedComponentGuid = m_SceneRenderer->GetGuidAtScreenPosition(ClickPos);
			LOG( LogViewportPanel, Info, "clicked on viewport at: %s\n\t found component with guid: %s", ClickPos.ToString().c_str(), SelectedComponentGuid.ToString().c_str() );

			Component* Comp = m_SceneRenderer->GetScene()->GetWorld()->GetComponentWithGuid(SelectedComponentGuid);
			if (Comp && Comp->GetOwningActor() && !Comp->GetOwningActor()->IsMarkedPendingKill())
			{
				LOG( LogViewportPanel, Info, "clicked on actor: %s", Comp->GetOwningActor()->GetActorLabel().c_str() );
			}
		}

		const ImVec2 RectMin = ImGui::GetItemRectMin();
		const ImVec2 RectMax = ImGui::GetItemRectMax();

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(RectMin.x, RectMin.y, RectMax.x - RectMin.x, RectMax.y - RectMin.y);

		float aspectRatio = (float) (m_SceneRenderer->GetViewportSize().X) / m_SceneRenderer->GetViewportSize().Y;
		
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		
		m_SceneRenderer->m_CameraActor->GetCameraComponent()->CalculateMatrices(viewMatrix, projectionMatrix, aspectRatio);

		XMFLOAT4X4 V;
		XMStoreFloat4x4(&V, viewMatrix);

		XMFLOAT4X4 P;
		XMStoreFloat4x4(&P, projectionMatrix);

		XMFLOAT4X4 M;
		XMStoreFloat4x4(&M, Mat);

		ImGuizmo::Manipulate( &V.m[0][0], &P.m[0][0], IMGUIZMO_NAMESPACE::TRANSLATE, IMGUIZMO_NAMESPACE::LOCAL, &M.m[0][0] );

		if (ImGuizmo::IsUsing())
		{
			Mat = XMLoadFloat4x4(&M);
		}
	}

	void ViewportPanel::SetRenderingEnabled( bool Enabled )
	{
		m_SceneRenderer->SetRenderingEnabled(Enabled);
	}

	void ViewportPanel::OnViewportSizeChanged( const IntPoint& NewSize )
	{
		//Renderer::Get()->MainSceneRenderer->ResizeView( NewSize );
		m_SceneRenderer->ResizeView( NewSize );

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};

		descSRV.Texture2D.MipLevels       = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
		descSRV.ViewDimension             = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping   = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		//Renderer::Get()->GetDevice()->GetD3D12Device()->CreateShaderResourceView( Renderer::Get()->MainSceneRenderer->GetViewResource(), &descSRV, ViewCpuHandle );
		Renderer::Get()->GetD3D12Device()->CreateShaderResourceView( m_SceneRenderer->GetViewResource(), &descSRV, ViewCpuHandle );
	}

}

#endif