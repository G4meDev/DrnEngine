#include "DrnPCH.h"
#include "Renderer.h"
#include "D3D12Scene.h"

#include "Runtime/Renderer/ImGui/ImGuiRenderer.h"

namespace Drn
{
	Renderer* Renderer::SingletonInstance;

	void Renderer::CreateMainScene()
	{
		//Get()->MainScene = new D3D12Scene(Adapter, GetMainWindow()->GetWindowHandle(), IntPoint(GetMainWindow()->GetSizeX(), GetMainWindow()->GetSizeY()), false, DISPLAY_OUTPUT_FORMAT);
	}

	Renderer* Renderer::Get()
	{
		return SingletonInstance;
	}

	void Renderer::Init(HINSTANCE inhInstance)
	{
		SingletonInstance = new Renderer();
	}

	void Renderer::Shutdown()
	{
		std::cout << "Renderer shutdown!" << std::endl;
	}

	void Renderer::Tick(float DeltaTime)
	{

	}
}