#include "DrnPCH.h"
#include "ViewportPanel.h"

#if WITH_EDITOR

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"
#include "Runtime/Renderer/Renderer.h"

LOG_DEFINE_CATEGORY( LogViewportPanel, "ViewportPanel" );

namespace Drn
{
	ViewportPanel::ViewportPanel(Scene* InScene)
	{
		m_World = InScene->GetWorld();
		m_Scene = InScene;
		m_SceneRenderer = m_Scene->AllocateSceneRenderer();
		m_SceneRenderer->OnPickedComponent.Add( this, &ViewportPanel::OnRendererPickedComponent );

		m_ViewportCamera = m_World->SpawnActor<CameraActor>();
		m_ViewportCamera->SetActorLocation(XMVectorSet(20, 28, 20, 0));

		XMVECTOR CameraRotation = XMQuaternionRotationRollPitchYaw(Math::PI / 4, Math::PI * 5 / 4, 0);
		m_ViewportCamera->SetActorRotation( CameraRotation );

		m_ViewportCamera->SetActorLabel( "ViewportCamera" );
		m_ViewportCamera->SetTransient(true);

		m_SceneRenderer->m_CameraActor = m_ViewportCamera;

		//ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Alloc( &ViewCpuHandle, &ViewGpuHandle );
		Renderer::Get()->TempSRVAllocator.Alloc( &ViewCpuHandle, &ViewGpuHandle );

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
		//ImGuiRenderer::g_pd3dSrvDescHeapAlloc.Free(ViewCpuHandle, ViewGpuHandle);
		Renderer::Get()->TempSRVAllocator.Free(ViewCpuHandle, ViewGpuHandle);
	}

	void ViewportPanel::Draw( float DeltaTime )
	{
		SCOPE_STAT(ViewportPanelDraw);

		if (CameraInputHandler.Tick(DeltaTime))
		{
			m_ViewportCamera->ApplyViewportInput(CameraInputHandler, CameraMovementSpeed, CameraRotationSpeed);
		}

		DrawHeader();

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

		HandleInputs();

		bool UsingGizmo = false;

		SceneComponent* SelectedSceneComponent = nullptr;
		if (GetSelectedComponentDel.IsBound())
		{
			SelectedSceneComponent = static_cast<SceneComponent*>(GetSelectedComponentDel.Execute());
		}

		if (SelectedSceneComponent && SelectedSceneComponent->GetOwningActor() &&
			!SelectedSceneComponent->GetOwningActor()->IsMarkedPendingKill())
		{
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
			Matrix SceneComponentWorldTransform = SelectedSceneComponent->GetWorldTransform();
			XMStoreFloat4x4(&M, SceneComponentWorldTransform.Get());

			XMFLOAT4X4 Iden;
			XMStoreFloat4x4(&Iden, Matrix::MatrixIdentity.Get());

			float Snap[3];
			m_GizmoState.GetSnapValue(Snap);
			ImGuizmo::Manipulate( &V.m[0][0], &P.m[0][0], m_GizmoState.GetGuizmoSpace(), m_GizmoState.GetGuizmoMode(), &M.m[0][0],
				nullptr, Snap[0] == 0 ? nullptr : Snap );

			UsingGizmo = ImGuizmo::IsUsing();
			if (UsingGizmo)
			{
				Transform SceneComponentWorldTransform = Transform( XMLoadFloat4x4(&M) );

				if (m_GizmoState.m_Space == EGizmoSpace::Scale)
				{
					SelectedSceneComponent->SetRelativeScale(SceneComponentWorldTransform.GetScale());
				}

				else
				{
					SelectedSceneComponent->SetWorldLocationAndRotation(SceneComponentWorldTransform.GetLocation(),
						SceneComponentWorldTransform.GetRotation());
				}
			}
		}

		if ( !UsingGizmo && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left) )
		{
			const ImVec2 RectMin = ImGui::GetItemRectMin();
			const ImVec2 MousePos = ImGui::GetMousePos();
			const IntPoint ClickPos = IntPoint( MousePos.x - RectMin.x, MousePos.y - RectMin.y );
			m_SceneRenderer->QueueMousePickEvent(ClickPos);
		}


	}

	void ViewportPanel::DrawHeader()
	{
		float CameraSpeed = CameraMovementSpeed * 100;
		ImGui::SetNextItemWidth(128);
		ImGui::SliderFloat( "CameraSpeed", &CameraSpeed, 0.1f, 10, "%.3f");
		CameraMovementSpeed = CameraSpeed / 100;
		ImGui::SameLine(0, 32);

		m_GizmoState.Draw();
	}

	void ViewportPanel::HandleInputs()
	{
		if ( ImGui::IsItemHovered() )
		{
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				m_GizmoState.HandleInput();
			}

			if (HandleInputDel.IsBound())
			{
				HandleInputDel.Execute();
			}
		}
	}

	void ViewportPanel::OnRendererPickedComponent( Component* PickedComponent )
	{
		// broadcast to others. e.g. level viewport
		OnSelectedNewComponent.Braodcast( PickedComponent );
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