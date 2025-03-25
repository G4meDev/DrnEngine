#pragma once

#if WITH_EDITOR

#include "ForwardTypes.h"
#include "Runtime/Renderer/ImGui/ImGuiLayer.h"

LOG_DECLARE_CATEGORY( LogFileImportMenu );

namespace Drn
{
	class FileImportMenu;

	class FileImportMenuGuiLayer : public ImGuiLayer
	{
	public:
		FileImportMenuGuiLayer(FileImportMenu* InFileImportMenu);
		~FileImportMenuGuiLayer();

	protected:
		virtual void Draw() override;
	
	private:
		FileImportMenu* m_FileImportMenu;
	};
}

#endif