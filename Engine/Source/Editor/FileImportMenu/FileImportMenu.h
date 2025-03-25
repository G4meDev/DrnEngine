#pragma once

#if WITH_EDITOR

namespace Drn
{
	class FileImportMenuGuiLayer;

	class FileImportMenu
	{
	public:
		FileImportMenu();
		~FileImportMenu();

	protected:
		std::unique_ptr<FileImportMenuGuiLayer> GuiLayer;

	private:
		
	};
}

#endif