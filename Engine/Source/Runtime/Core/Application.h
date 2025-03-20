#pragma once

#include <GameFramework/GameFramework.h>

#include <GameFramework/Window.h>
#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/SwapChain.h>

#include <dx12lib/VertexBuffer.h>
#include <dx12lib/IndexBuffer.h>

#include <dx12lib/RootSignature.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/Texture.h>

#include "GameFramework/HighResolutionTimer.h"
#include "GameFramework/Events.h"

namespace Drn
{
	class Application
	{
	public:
		virtual int Run(HINSTANCE inhInstance);

	protected:
		//virtual void Startup();
		//virtual void Shutdown();
		//
		//virtual void Tick(float DeltaTime);

		void OnUpdate( UpdateEventArgs& e );
		void OnKeyPressed( KeyEventArgs& e );
		void OnWindowResized( ResizeEventArgs& e );
		void OnWindowClose( WindowCloseEventArgs& e );


		bool bRunning = true;

		HINSTANCE m_hInstance;

		Window* m_MainWindow;

		HighResolutionTimer m_Timer;

		Logger logger;

		std::shared_ptr<dx12lib::Device> pDevice = nullptr;

		std::shared_ptr<Window> pGameWindow = nullptr;

		std::shared_ptr<dx12lib::SwapChain> pSwapChain = nullptr;

		std::shared_ptr<dx12lib::VertexBuffer> pVertexBuffer = nullptr;
		std::shared_ptr<dx12lib::IndexBuffer>  pIndexBuffer  = nullptr;

		std::shared_ptr<dx12lib::RootSignature> pRootSignature = nullptr;
		std::shared_ptr<dx12lib::PipelineStateObject>    pPipelineStateObject = nullptr;

		std::shared_ptr<dx12lib::Texture> pDepthTexture = nullptr;

		dx12lib::RenderTarget m_RenderTarget;

		float fieldOfView = 45.0f;
	};
}