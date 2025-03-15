project "ImGui"
	kind "StaticLib"
	language "C++"
    staticruntime "on"
	cppdialect "C++17"

	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	includedirs
	{
		"../ImGui",
		"../ThirdParty/Header"
	}
	
	files
	{
		"backends/imgui_impl_win32.h",
		"backends/imgui_impl_win32.cpp",
		"backends/imgui_impl_dx12.h",
		"backends/imgui_impl_dx12.cpp",
		"imgui_impl_opengl3_loader.h",
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_tables.cpp",
		"imgui_demo.cpp"
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"