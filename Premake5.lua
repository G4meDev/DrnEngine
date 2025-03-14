workspace "DrnEngine"
	architecture "x64"
	startproject "Game"

	configurations
	{
	"DebugEditor",
	"Debug",
	"Release"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	
----------------------------------------------
---------- Engine -----------------------
----------------------------------------------
	
project "Engine"
	location "Engine"
	kind "Staticlib"
	language "c++"
	cppdialect "c++17"
	staticruntime "on"
	
	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "DrnPCH.h"
	pchsource "Engine/Source/DrnPCH.cpp"

	files
	{
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
	}
	
	libdirs
	{
		"ThirdParty/Library/**"
	}
	
	includedirs
	{
		"%{prj.name}/Source",
		"ThirdParty/Header"
	}
	
	links
	{
		"d3d12.lib",
		"D3DCompiler.lib",
		"DXGI.lib",
		"dxguid.lib"
	}
	
	filter "configurations:DebugEditor"
		defines
		{
			"DRN_DEBUG",
			"WITH_EDITOR"
		}
			
		runtime "Debug"
		symbols "on"	

	filter "configurations:Debug"
		defines
		{
			"DRN_DEBUG"
		}
			
		runtime "Debug"
		symbols "on"			
		
	filter "configurations:Release"
		defines
		{
			"DRN_RELEASE"
		}
		
		runtime "Release"
		optimize "on"
				
	filter "configurations:Dist"
		defines
		{
			"DRN_DIST"
		}
		
		runtime "Release"
		optimize "on"


----------------------------------------------
---------- Game ---------------------------
----------------------------------------------
	
project "Game"
	location "Game"
	kind "WindowedApp"
	language "c++"
	cppdialect "c++17"
	staticruntime "on"
	
	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp"
	}
	
	includedirs
	{
		"Engine/Source"
	}
	
	links "Engine"
	
	filter "configurations:DebugEditor"
		defines
		{
			"DRN_DEBUG",
			"WITH_EDITOR"
		}
		runtime "Debug"
		symbols "on"	
	
	filter "configurations:Debug"
		defines
		{
			"DRN_DEBUG"
		}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines
		{
			"DRN_RELEASE"
		}
		runtime "Release"
		optimize "on"
	
	filter "configurations:Dist"
		defines
		{
			"DRN_DIST"
		}
		runtime "Release"
		optimize "on"