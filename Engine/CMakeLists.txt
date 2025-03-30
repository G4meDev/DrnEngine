cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

#file(GLOB_RECURSE SOURCE_FILES *.cpp)

set( HEADER_FILES
	Source/Runtime/Core/Drn.h
	Source/Runtime/Core/Application.h
	Source/Runtime/Core/Asset.h
	Source/Runtime/Core/AssetManager.h
	Source/Runtime/Core/Archive.h
	Source/Runtime/Core/Serializable.h
	
	Source/Runtime/Engine/World.h
	Source/Runtime/Engine/Scene.h
	Source/Runtime/Engine/PrimitiveComponent.h
	Source/Runtime/Engine/StaticMeshComponent.h
	Source/Runtime/Engine/StaticMesh.h
	
	Source/Runtime/Renderer/Renderer.h
	Source/Runtime/Renderer/SceneRenderer.h
	Source/Runtime/Renderer/D3D12Utils.h
	Source/Runtime/Renderer/ImGui/ImGuiLayer.h
	Source/Runtime/Renderer/ImGui/ImGuiRenderer.h
	
	Source/Runtime/Containers/List.h
	
	Source/Runtime/Math/IntPoint.h
	Source/Runtime/Math/Math.h
	
	Source/Runtime/Misc/StringHelper.h
	Source/Runtime/Misc/FileSystem.h
	Source/Runtime/Misc/DateTime.h
	Source/Runtime/Misc/EnumClassFlags.h
	Source/Runtime/Misc/Log.h
	Source/Runtime/Misc/Path.h
	
	Source/Editor/Editor.h
	
	Source/Editor/Viewport/Viewport.h
	Source/Editor/Viewport/ViewportGuiLayer.h
	
	Source/Editor/OutputLog/OutputLog.h
	Source/Editor/OutputLog/OutputLogGuiLayer.h

	Source/Editor/ContentBrowser/ContentBrowser.h
	Source/Editor/ContentBrowser/ContentBrowserGuiLayer.h

	Source/Editor/AssetPreview/AssetPreviewStaticMeshGuiLayer.h

	Source/Editor/AssetImporter/AssetImporterStaticMesh.h

	Source/Editor/FileImportMenu/FileImportMenu.h
	Source/Editor/FileImportMenu/FileImportMenuGuiLayer.h
)

set( SOURCE_FILES
	Source/DrnPCH.h
	Source/DrnPCH.cpp
	Source/ForwardTypes.h
	
	Source/Runtime/Core/Asset.cpp
	Source/Runtime/Core/AssetManager.cpp
	Source/Runtime/Core/Application.cpp
	Source/Runtime/Core/Archive.cpp
	Source/Runtime/Core/Serializable.cpp
	
	Source/Runtime/Engine/World.cpp
	Source/Runtime/Engine/Scene.cpp
	Source/Runtime/Engine/PrimitiveComponent.cpp
	Source/Runtime/Engine/StaticMeshComponent.cpp
	Source/Runtime/Engine/StaticMesh.cpp
	
	Source/Runtime/Renderer/Renderer.cpp
	Source/Runtime/Renderer/SceneRenderer.cpp
	Source/Runtime/Renderer/D3D12Utils.cpp
	Source/Runtime/Renderer/ImGui/ImGuiLayer.cpp
	Source/Runtime/Renderer/ImGui/ImGuiRenderer.cpp
	
	Source/Runtime/Containers/List.cpp
	
	Source/Runtime/Math/IntPoint.cpp
	Source/Runtime/Math/Math.cpp
	
	Source/Runtime/Misc/StringHelper.cpp
	Source/Runtime/Misc/FileSystem.cpp
	Source/Runtime/Misc/DateTime.cpp
	Source/Runtime/Misc/EnumClassFlags.cpp
	Source/Runtime/Misc/Log.cpp
	Source/Runtime/Misc/Path.cpp
	
	Source/Editor/Editor.cpp
	
	Source/Editor/Viewport/Viewport.cpp
	Source/Editor/Viewport/ViewportGuiLayer.cpp
	
	Source/Editor/OutputLog/OutputLog.cpp
	Source/Editor/OutputLog/OutputLogGuiLayer.cpp

	Source/Editor/ContentBrowser/ContentBrowser.cpp
	Source/Editor/ContentBrowser/ContentBrowserGuiLayer.cpp

	Source/Editor/AssetPreview/AssetPreviewStaticMeshGuiLayer.cpp

	Source/Editor/AssetImporter/AssetImporterStaticMesh.cpp

	Source/Editor/FileImportMenu/FileImportMenu.cpp
	Source/Editor/FileImportMenu/FileImportMenuGuiLayer.cpp
)

set( SHADER_FILES_VS
    Content/Shaders/TestShader_VS.hlsl
)

set( SHADER_FILES_PS
    Content/Shaders/TestShader_PS.hlsl
)

set( SHADER_FILES
    ${SHADER_FILES_VS}
    ${SHADER_FILES_PS}
)

source_group( "Content\\Shaders" FILES ${SHADER_FILES_VS} ${SHADER_FILES_PS} )

add_library( Engine STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${SHADER_FILES}
)

# Enable C++17 compiler features.
target_compile_features( Engine 
	PUBLIC cxx_std_17
)

target_compile_definitions( Engine
    PUBLIC NOMINMAX
)

add_compile_definitions("$<$<CONFIG:Debug>:WITH_EDITOR>")

target_include_directories( Engine
    PUBLIC inc Source/
    PUBLIC inc ../ThirdParty/ImGui/
    PUBLIC inc ../ThirdParty/DX12Lib/inc/
    PUBLIC inc ../ThirdParty/ImGuiFileDialog/
    PUBLIC inc ../GameFramework/inc/
	PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders"
)

target_link_libraries( Engine 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
	PUBLIC d3dcompiler.lib
	PUBLIC ImGui
    PUBLIC assimp
    PUBLIC DirectXTex
    PUBLIC gainput
    PUBLIC DX12Lib
    PUBLIC GameFramework
    PUBLIC ImGuiFileDialog
)

# Enable precompiled header files.
set_source_files_properties( ${SOURCE_FILES} 
    PROPERTIES 
        COMPILE_FLAGS /Yu"DrnPCH.h"
)

set_source_files_properties( Source/DrnPCH.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"DrnPCH.h"
)

set_source_files_properties( ${SHADER_FILES_PS}
    PROPERTIES
        VS_SHADER_MODEL 6.0
        VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders/%(Filename).h"
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
        VS_SHADER_TYPE Pixel
)

set_source_files_properties( ${SHADER_FILES_VS}
    PROPERTIES
        VS_SHADER_MODEL 6.0
        VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders/%(Filename).h"
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
        VS_SHADER_TYPE Vertex
)

set_source_files_properties( Content/Shaders/TestShader_VS.hlsl
    PROPERTIES
        VS_SHADER_VARIABLE_NAME TestShader_VS
)

set_source_files_properties( Content/Shaders/TestShader_PS.hlsl
    PROPERTIES
        VS_SHADER_VARIABLE_NAME TestShader_PS
)