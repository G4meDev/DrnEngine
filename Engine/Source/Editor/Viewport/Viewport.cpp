#include "DrnPCH.h"
#include "Viewport.h"

#if WITH_EDITOR

#include "Editor/Viewport/ViewportGuiLayer.h"
#include "Runtime/Renderer/Renderer.h"

namespace Drn
{
	std::unique_ptr<Viewport> Viewport::SingletonInstance;

	Viewport::Viewport()
	{

	}

	Viewport::~Viewport()
	{

	}

	void Viewport::Init()
	{
		ViewportLayer = std::make_unique<ViewportGuiLayer>();
		ViewportLayer->Attach();
	}

	void Viewport::Tick(float DeltaTime)
	{

	}

	Viewport* Viewport::Get()
	{
		if (!SingletonInstance.get())
		{
			SingletonInstance = std::make_unique<Viewport>();
		}

		return SingletonInstance.get();
	}

	void Viewport::OnViewportSizeChanged(const IntPoint& NewSize)
	{

	}

}

#endif