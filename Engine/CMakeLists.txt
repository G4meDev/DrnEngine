cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

#file(GLOB_RECURSE SOURCE_FILES *.cpp)

set( HEADER_FILES
	Source/Runtime/Core/Drn.h
	Source/Runtime/Core/Application.h
	Source/Runtime/Core/Taskflow.h
	Source/Runtime/Core/Asset.h
	Source/Runtime/Core/AssetManager.h
	Source/Runtime/Core/Archive.h
	Source/Runtime/Core/FileArchive.h
	Source/Runtime/Core/BufferArchive.h
	Source/Runtime/Core/Serializable.h
	Source/Runtime/Core/Delegate.h
	Source/Runtime/Core/Guid.h
	Source/Runtime/Core/Time.h
	Source/Runtime/Core/Window.h
	
	Source/Runtime/Engine/EngineTypes.h
	Source/Runtime/Engine/WorldManager.h
	Source/Runtime/Engine/World.h
	Source/Runtime/Engine/Level.h
	Source/Runtime/Engine/Scene.h
	Source/Runtime/Engine/PrimitiveComponent.h
	Source/Runtime/Engine/PrimitiveSceneProxy.h
	Source/Runtime/Engine/StaticMeshComponent.h
	Source/Runtime/Engine/StaticMesh.h
	Source/Runtime/Engine/StaticMeshVertexData.h
	Source/Runtime/Engine/NamedProperty.h
	Source/Runtime/Engine/MeshTypes.h
	Source/Runtime/Engine/CameraComponent.h
	Source/Runtime/Engine/Component.h
	Source/Runtime/Engine/SceneComponent.h
	Source/Runtime/Engine/Actor.h
	Source/Runtime/Engine/StaticMeshActor.h
	Source/Runtime/Engine/CameraActor.h
	Source/Runtime/Engine/PointLightActor.h
	Source/Runtime/Engine/SpotLightActor.h
	Source/Runtime/Engine/LightSceneProxy.h
	Source/Runtime/Engine/LightActor.h
	Source/Runtime/Engine/PointLightSceneProxy.h
	Source/Runtime/Engine/SpotLightSceneProxy.h
	Source/Runtime/Engine/DirectionalLightSceneProxy.h
	Source/Runtime/Engine/DirectionalLightActor.h
	Source/Runtime/Engine/SkyLightActor.h
	Source/Runtime/Engine/SkyLightSceneProxy.h
	Source/Runtime/Engine/PostProcessSettings.h
	Source/Runtime/Engine/PostProcessVolume.h
	Source/Runtime/Engine/InputManager.h
	Source/Runtime/Engine/CameraTypes.h

	Source/Runtime/GameFramework/Pawn.h
	Source/Runtime/GameFramework/Character.h
	Source/Runtime/GameFramework/Controller.h
	Source/Runtime/GameFramework/PlayerController.h
	Source/Runtime/GameFramework/CameraManager.h
	
	Source/Runtime/Renderer/Renderer.h
	Source/Runtime/Renderer/CommonResources.h
	Source/Runtime/Renderer/D3D12CommandList.h
	Source/Runtime/Renderer/ShaderBlob.h
	Source/Runtime/Renderer/SceneRenderer.h
	Source/Runtime/Renderer/RenderBuffer/RenderBuffer.h
	Source/Runtime/Renderer/RenderBuffer/HitProxyRenderBuffer.h
	Source/Runtime/Renderer/RenderBuffer/GBuffer.h
	Source/Runtime/Renderer/RenderBuffer/TonemapRenderBuffer.h
	Source/Runtime/Renderer/RenderBuffer/EditorPrimitiveRenderBuffer.h
	Source/Runtime/Renderer/RenderBuffer/EditorSelectionRenderBuffer.h
	Source/Runtime/Renderer/RenderBuffer/HZBBuffer.h
	Source/Runtime/Renderer/RenderBuffer/RenderBufferAO.h
	Source/Runtime/Renderer/RenderBuffer/ScreenSpaceReflectionBuffer.h
	Source/Runtime/Renderer/RenderBuffer/ReflectionEnvironmentBuffer.h
	Source/Runtime/Renderer/RenderBuffer/TAABuffer.h
	Source/Runtime/Renderer/RenderBuffer/SceneDownSampleBuffer.h
	Source/Runtime/Renderer/RenderBuffer/BloomBuffer.h
	Source/Runtime/Renderer/Device.h
	Source/Runtime/Renderer/SwapChain.h
	Source/Runtime/Renderer/BufferedResource.h
	Source/Runtime/Renderer/Resource.h
	Source/Runtime/Renderer/PipelineStateObject.h
	Source/Runtime/Renderer/InputLayout.h
	Source/Runtime/Renderer/StaticMeshSceneProxy.h
	Source/Runtime/Renderer/D3D12Utils.h
	Source/Runtime/Renderer/ImGui/ImGuiLayer.h
	Source/Runtime/Renderer/ImGui/ImGuiRenderer.h
	Source/Runtime/Renderer/Texture/Texture.h
	Source/Runtime/Renderer/Texture/Texture2D.h
	Source/Runtime/Renderer/Texture/TextureCube.h
	Source/Runtime/Renderer/Texture/TextureVolume.h
	Source/Runtime/Renderer/Texture/TextureHelper.h
	Source/Runtime/Renderer/Material/Material.h
	Source/Runtime/Renderer/VertexBuffer.h
	Source/Runtime/Renderer/IndexBuffer.h
	Source/Runtime/Renderer/RenderGeometeryHelper.h
	Source/Runtime/Renderer/ResourceStateTracker.h
	Source/Runtime/Renderer/ResourceView.h

	Source/Runtime/Components/LineBatchComponent.h
	Source/Runtime/Components/LightComponent.h
	Source/Runtime/Components/PointLightComponent.h
	Source/Runtime/Components/SpotLightComponent.h
	Source/Runtime/Components/DirectionalLightComponent.h
	Source/Runtime/Components/SkyLightComponent.h
	Source/Runtime/Components/BillboardComponent.h
	Source/Runtime/Components/InputComponent.h
	Source/Runtime/Components/CharacterMovementComponent.h
	Source/Runtime/Components/SpringArmComponent.h

	Source/Runtime/Physic/PhysicCore.h
	Source/Runtime/Physic/PhysicManager.h
	Source/Runtime/Physic/PhysicScene.h
	Source/Runtime/Physic/BodyInstance.h
	Source/Runtime/Physic/BodySetup.h
	Source/Runtime/Physic/AggregateGeom.h
	Source/Runtime/Physic/ShapeElem.h
	Source/Runtime/Physic/SphereElem.h
	Source/Runtime/Physic/BoxElem.h
	Source/Runtime/Physic/CapsuleElem.h
	Source/Runtime/Physic/PhysicUserData.h
	
	Source/Runtime/Containers/List.h

	Source/Runtime/Math/Math.h
	Source/Runtime/Math/MathHelper.h
	Source/Runtime/Math/IntPoint.h
	Source/Runtime/Math/Box.h
	Source/Runtime/Math/Sphere.h
	Source/Runtime/Math/Vector.h
	Source/Runtime/Math/Vector4.h
	Source/Runtime/Math/Vector2.h
	Source/Runtime/Math/Quat.h
	Source/Runtime/Math/Transform.h
	Source/Runtime/Math/Rotator.h
	Source/Runtime/Math/Matrix.h
	Source/Runtime/Math/Color.h
	
	Source/Runtime/Misc/StringHelper.h
	Source/Runtime/Misc/FileSystem.h
	Source/Runtime/Misc/DateTime.h
	Source/Runtime/Misc/EnumClassFlags.h
	Source/Runtime/Misc/Log.h
	Source/Runtime/Misc/Path.h
	Source/Runtime/Misc/StatsMisc.h
	
	Source/Editor/Misc/ViewportCameraInputHandler.h
	Source/Editor/Misc/GizmoState.h

	Source/Editor/Editor.h
	Source/Editor/EditorConfig.h
	
	Source/Editor/EditorPanels/ViewportPanel.h
	Source/Editor/EditorPanels/ActorDetailPanel.h
	Source/Editor/EditorPanels/WorldOutlinerPanel.h
	Source/Editor/EditorPanels/ModesPanel.h

	Source/Editor/LevelViewport/LevelViewport.h
	Source/Editor/LevelViewport/LevelViewportGuiLayer.h
	
	Source/Editor/OutputLog/OutputLog.h
	Source/Editor/OutputLog/OutputLogGuiLayer.h

	Source/Editor/Misc/TaskGraphVisualizer.h
	Source/Editor/Misc/EditorMisc.h

	Source/Editor/ContentBrowser/ContentBrowser.h
	Source/Editor/ContentBrowser/ContentBrowserGuiLayer.h

	Source/Editor/AssetPreview/AssetPreviewStaticMeshGuiLayer.h
	Source/Editor/AssetPreview/AssetPreviewMaterialGuiLayer.h
	Source/Editor/AssetPreview/AssetPreviewTexture2DGuiLayer.h
	Source/Editor/AssetPreview/AssetPreviewTextureCubeGuiLayer.h

	Source/Editor/AssetImporter/AssetImporterStaticMesh.h
	Source/Editor/AssetImporter/AssetImporterMaterial.h
	Source/Editor/AssetImporter/AssetImporterTexture.h

	Source/Editor/FileImportMenu/FileImportMenu.h
	Source/Editor/FileImportMenu/FileImportMenuGuiLayer.h
	
	Source/ThirdParty/par/par_shapes.h

)

set( SOURCE_FILES
	Source/DrnPCH.h
	Source/DrnPCH.cpp
	Source/ForwardTypes.h
	
	Source/Runtime/Core/Taskflow.cpp
	Source/Runtime/Core/Asset.cpp
	Source/Runtime/Core/AssetManager.cpp
	Source/Runtime/Core/Application.cpp
	Source/Runtime/Core/Archive.cpp
	Source/Runtime/Core/FileArchive.cpp
	Source/Runtime/Core/BufferArchive.cpp
	Source/Runtime/Core/Serializable.cpp
	Source/Runtime/Core/Delegate.cpp
	Source/Runtime/Core/Guid.cpp
	Source/Runtime/Core/Time.cpp
	Source/Runtime/Core/Window.cpp
	
	Source/Runtime/Engine/EngineTypes.cpp
	Source/Runtime/Engine/WorldManager.cpp
	Source/Runtime/Engine/World.cpp
	Source/Runtime/Engine/Level.cpp
	Source/Runtime/Engine/Scene.cpp
	Source/Runtime/Engine/PrimitiveComponent.cpp
	Source/Runtime/Engine/PrimitiveSceneProxy.cpp
	Source/Runtime/Engine/StaticMeshComponent.cpp
	Source/Runtime/Engine/StaticMesh.cpp
	Source/Runtime/Engine/StaticMeshVertexData.cpp
	Source/Runtime/Engine/MeshTypes.cpp
	Source/Runtime/Engine/NamedProperty.cpp
	Source/Runtime/Engine/CameraComponent.cpp
	Source/Runtime/Engine/Component.cpp
	Source/Runtime/Engine/SceneComponent.cpp
	Source/Runtime/Engine/Actor.cpp
	Source/Runtime/Engine/StaticMeshActor.cpp
	Source/Runtime/Engine/CameraActor.cpp
	Source/Runtime/Engine/LightActor.cpp
	Source/Runtime/Engine/PointLightActor.cpp
	Source/Runtime/Engine/SpotLightActor.cpp

	Source/Runtime/Engine/LightSceneProxy.cpp
	Source/Runtime/Engine/PointLightSceneProxy.cpp
	Source/Runtime/Engine/SpotLightSceneProxy.cpp
	Source/Runtime/Engine/DirectionalLightActor.cpp
	Source/Runtime/Engine/DirectionalLightSceneProxy.cpp
	Source/Runtime/Engine/SkyLightActor.cpp
	Source/Runtime/Engine/SkyLightSceneProxy.cpp
	Source/Runtime/Engine/PostProcessSettings.cpp
	Source/Runtime/Engine/PostProcessVolume.cpp
	Source/Runtime/Engine/InputManager.cpp
	Source/Runtime/Engine/CameraTypes.cpp
	
	Source/Runtime/GameFramework/Pawn.cpp
	Source/Runtime/GameFramework/Character.cpp
	Source/Runtime/GameFramework/Controller.cpp
	Source/Runtime/GameFramework/PlayerController.cpp
	Source/Runtime/GameFramework/CameraManager.cpp

	Source/Runtime/Renderer/Renderer.cpp
	Source/Runtime/Renderer/CommonResources.cpp
	Source/Runtime/Renderer/D3D12CommandList.cpp
	Source/Runtime/Renderer/ShaderBlob.cpp
	Source/Runtime/Renderer/SceneRenderer.cpp
	Source/Runtime/Renderer/RenderBuffer/RenderBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/HitProxyRenderBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/GBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/TonemapRenderBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/EditorPrimitiveRenderBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/EditorSelectionRenderBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/HZBBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/RenderBufferAO.cpp
	Source/Runtime/Renderer/RenderBuffer/ScreenSpaceReflectionBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/ReflectionEnvironmentBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/TAABuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/SceneDownSampleBuffer.cpp
	Source/Runtime/Renderer/RenderBuffer/BloomBuffer.cpp
	Source/Runtime/Renderer/Device.cpp
	Source/Runtime/Renderer/SwapChain.cpp
	Source/Runtime/Renderer/BufferedResource.cpp
	Source/Runtime/Renderer/Resource.cpp
	Source/Runtime/Renderer/PipelineStateObject.cpp
	Source/Runtime/Renderer/InputLayout.cpp
	Source/Runtime/Renderer/StaticMeshSceneProxy.cpp
	Source/Runtime/Renderer/D3D12Utils.cpp
	Source/Runtime/Renderer/ImGui/ImGuiLayer.cpp
	Source/Runtime/Renderer/ImGui/ImGuiRenderer.cpp
	Source/Runtime/Renderer/Texture/Texture.cpp
	Source/Runtime/Renderer/Texture/Texture2D.cpp
	Source/Runtime/Renderer/Texture/TextureCube.cpp
	Source/Runtime/Renderer/Texture/TextureVolume.cpp
	Source/Runtime/Renderer/Texture/TextureHelper.cpp
	Source/Runtime/Renderer/Material/Material.cpp
	Source/Runtime/Renderer/VertexBuffer.cpp
	Source/Runtime/Renderer/IndexBuffer.cpp
	Source/Runtime/Renderer/RenderGeometeryHelper.cpp
	Source/Runtime/Renderer/ResourceStateTracker.cpp
	Source/Runtime/Renderer/ResourceView.cpp

	Source/Runtime/Components/LineBatchComponent.cpp
	Source/Runtime/Components/LightComponent.cpp
	Source/Runtime/Components/PointLightComponent.cpp
	Source/Runtime/Components/SpotLightComponent.cpp
	Source/Runtime/Components/DirectionalLightComponent.cpp
	Source/Runtime/Components/SkyLightComponent.cpp
	Source/Runtime/Components/BillboardComponent.cpp
	Source/Runtime/Components/InputComponent.cpp
	Source/Runtime/Components/CharacterMovementComponent.cpp
	Source/Runtime/Components/SpringArmComponent.cpp

	Source/Runtime/Physic/PhysicCore.cpp
	Source/Runtime/Physic/PhysicManager.cpp
	Source/Runtime/Physic/PhysicScene.cpp
	Source/Runtime/Physic/BodyInstance.cpp
	Source/Runtime/Physic/AggregateGeom.cpp
	Source/Runtime/Physic/BodySetup.cpp
	Source/Runtime/Physic/ShapeElem.cpp
	Source/Runtime/Physic/SphereElem.cpp
	Source/Runtime/Physic/BoxElem.cpp
	Source/Runtime/Physic/CapsuleElem.cpp
	Source/Runtime/Physic/PhysicUserData.cpp
	
	Source/Runtime/Containers/List.cpp
	
	Source/Runtime/Math/Math.cpp
	Source/Runtime/Math/MathHelper.cpp
	Source/Runtime/Math/IntPoint.cpp
	Source/Runtime/Math/Box.cpp
	Source/Runtime/Math/Sphere.cpp
	Source/Runtime/Math/Vector.cpp
	Source/Runtime/Math/Vector4.cpp
	Source/Runtime/Math/Vector2.cpp
	Source/Runtime/Math/Quat.cpp
	Source/Runtime/Math/Transform.cpp
	Source/Runtime/Math/Rotator.cpp
	Source/Runtime/Math/Matrix.cpp
	Source/Runtime/Math/Color.cpp
	
	Source/Runtime/Misc/StringHelper.cpp
	Source/Runtime/Misc/FileSystem.cpp
	Source/Runtime/Misc/DateTime.cpp
	Source/Runtime/Misc/EnumClassFlags.cpp
	Source/Runtime/Misc/Log.cpp
	Source/Runtime/Misc/Path.cpp
	Source/Runtime/Misc/StatsMisc.cpp
	
	Source/Editor/Editor.cpp
	Source/Editor/EditorConfig.cpp

	Source/Editor/EditorPanels/ViewportPanel.cpp
	Source/Editor/EditorPanels/ActorDetailPanel.cpp
	Source/Editor/EditorPanels/WorldOutlinerPanel.cpp
	Source/Editor/EditorPanels/ModesPanel.cpp

	Source/Editor/LevelViewport/LevelViewport.cpp
	Source/Editor/LevelViewport/LevelViewportGuiLayer.cpp

	Source/Editor/Misc/ViewportCameraInputHandler.cpp
	Source/Editor/Misc/GizmoState.cpp
	Source/Editor/Misc/EditorMisc.cpp
	
	Source/Editor/OutputLog/OutputLog.cpp
	Source/Editor/OutputLog/OutputLogGuiLayer.cpp

	Source/Editor/ContentBrowser/ContentBrowser.cpp
	Source/Editor/ContentBrowser/ContentBrowserGuiLayer.cpp

	Source/Editor/Misc/TaskGraphVisualizer.cpp

	Source/Editor/AssetPreview/AssetPreviewStaticMeshGuiLayer.cpp
	Source/Editor/AssetPreview/AssetPreviewMaterialGuiLayer.cpp
	Source/Editor/AssetPreview/AssetPreviewTexture2DGuiLayer.cpp
	Source/Editor/AssetPreview/AssetPreviewTextureCubeGuiLayer.cpp

	Source/Editor/AssetImporter/AssetImporterStaticMesh.cpp
	Source/Editor/AssetImporter/AssetImporterMaterial.cpp
	Source/Editor/AssetImporter/AssetImporterTexture.cpp

	Source/Editor/FileImportMenu/FileImportMenu.cpp
	Source/Editor/FileImportMenu/FileImportMenuGuiLayer.cpp
)

set( OPTICK_HEADERS
	Source/ThirdParty/Optick/optick.h
	Source/ThirdParty/Optick/optick.config.h
	Source/ThirdParty/Optick/optick_capi.h
	Source/ThirdParty/Optick/optick_common.h
	Source/ThirdParty/Optick/optick_core.h
	Source/ThirdParty/Optick/optick_core.platform.h
	Source/ThirdParty/Optick/optick_core.win.h
	Source/ThirdParty/Optick/optick_gpu.h
	Source/ThirdParty/Optick/optick_memory.h
	Source/ThirdParty/Optick/optick_message.h
	Source/ThirdParty/Optick/optick_miniz.h
	Source/ThirdParty/Optick/optick_serialization.h
	Source/ThirdParty/Optick/optick_server.h
)

set( OPTICK_SOURCES
	Source/ThirdParty/Optick/optick_capi.cpp
	Source/ThirdParty/Optick/optick_core.cpp
	Source/ThirdParty/Optick/optick_gpu.cpp
	Source/ThirdParty/Optick/optick_gpu.d3d12.cpp
	Source/ThirdParty/Optick/optick_message.cpp
	Source/ThirdParty/Optick/optick_miniz.cpp
	Source/ThirdParty/Optick/optick_serialization.cpp
	Source/ThirdParty/Optick/optick_server.cpp
)

#set( SHADER_FILES_VS
#    Content/Shader/DebugLineShader.hlsl
#)
#
#set( SHADER_FILES_PS
#    Content/Shader/DebugLineShader.hlsl
#)
#
#set( SHADER_FILES
#    ${SHADER_FILES_VS}
#    ${SHADER_FILES_PS}
#)
#
#source_group( "Content\\Shaders" FILES ${SHADER_FILES_VS} ${SHADER_FILES_PS} )

#set( SHADER_FILES
#    Content/Materials/Common.hlsl
#)

file(GLOB_RECURSE SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Content/*.hlsl")

source_group( "Content\\Shaders" FILES ${SHADER_FILES} )

add_library( Engine STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${SHADER_FILES}
    ${OPTICK_HEADERS}
    ${OPTICK_SOURCES}
)

# Enable C++17 compiler features.
target_compile_features( Engine 
	PUBLIC cxx_std_23
)

target_compile_definitions( Engine
    PUBLIC NOMINMAX
)

add_compile_definitions(PX_PHYSX_STATIC_LIB)
add_compile_definitions($<$<CONFIG:Debug>:WITH_EDITOR>)
add_compile_definitions($<$<CONFIG:Debug>:WITH_PVD>)
add_compile_definitions($<$<CONFIG:Debug>:WITH_DEBUG>)
add_compile_definitions($<$<CONFIG:RelWithDebInfo>:WITH_DEBUG>)
add_compile_definitions($<$<CONFIG:Debug>:USE_PIX>)
add_compile_definitions($<$<CONFIG:RelWithDebInfo>:USE_PIX>)

target_include_directories( Engine
    PUBLIC inc Source/
	PUBLIC inc Source/ThirdParty/
    PUBLIC inc ../ThirdParty/ImGui/
    PUBLIC inc ../ThirdParty/ImGuiFileDialog/
    PUBLIC inc ../ThirdParty/ImGuizmo/
    PUBLIC inc ../ThirdParty/Physx/include/
    PUBLIC inc ../ThirdParty/PixEvent/include/
    PUBLIC inc ../ThirdParty/OneTBB/include/
    PUBLIC inc ../ThirdParty/Taskflow/
    PUBLIC inc ../ThirdParty/DXC/inc
    PUBLIC inc ../ThirdParty/Renderdoc/inc
    PUBLIC inc ../ThirdParty/lz4/lib
	#PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders"
)

target_link_directories( Engine
	PUBLIC ../ThirdParty/Physx/lib/
	PUBLIC ../ThirdParty/PixEvent/lib/
	PUBLIC ../ThirdParty/OneTBB/lib/
	PUBLIC ../ThirdParty/Optick/lib/
	PUBLIC ../ThirdParty/DXC/lib/
)

target_link_libraries( Engine 
    PUBLIC d3d12.lib
    PUBLIC d3d11.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
	PUBLIC d3dcompiler.lib
	PUBLIC shlwapi.lib
    PUBLIC WinPixEventRuntime.lib
	PUBLIC lz4_static
	PUBLIC ImGui
    PUBLIC assimp
    PUBLIC DirectXTex
    PUBLIC gainput
    PUBLIC ImGuiFileDialog
    PUBLIC ImGuizmo
	
    PUBLIC PhysX_static_64.lib
    PUBLIC PhysXCharacterKinematic_static_64.lib
    PUBLIC PhysXCommon_static_64.lib
    PUBLIC PhysXCooking_static_64.lib
    PUBLIC PhysXExtensions_static_64.lib
    PUBLIC PhysXFoundation_static_64.lib
    PUBLIC PhysXPvdSDK_static_64.lib
    PUBLIC PhysXVehicle2_static_64.lib
    
	PUBLIC dxcompiler.lib
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

add_custom_command(TARGET Engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/ThirdParty/PixEvent/redist/$(Configuration)
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
)

add_custom_command(TARGET Engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/ThirdParty/Renderdoc/lib/
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
)

add_custom_command(TARGET Engine POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/ThirdParty/DXC/bin/
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>
)

set_source_files_properties( ${SHADER_FILES}
    PROPERTIES
        VS_SHADER_MODEL 6.2
		VS_SETTINGS "ExcludedFromBuild=true"
)

#set_source_files_properties( ${SHADER_FILES_PS}
#    PROPERTIES
#        VS_SHADER_MODEL 6.0
#        VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders/%(Filename).h"
#        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
#        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
#        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
#        VS_SHADER_TYPE Pixel
#)
#
#set_source_files_properties( ${SHADER_FILES_VS}
#    PROPERTIES
#        VS_SHADER_MODEL 6.0
#        VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders/%(Filename).h"
#        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
#        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
#        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
#        VS_SHADER_TYPE Vertex
#)
#
#set_source_files_properties( Content/Shaders/TestShader_VS.hlsl
#    PROPERTIES
#        VS_SHADER_VARIABLE_NAME TestShader_VS
#)
#
#set_source_files_properties( Content/Shaders/TestShader_PS.hlsl
#    PROPERTIES
#        VS_SHADER_VARIABLE_NAME TestShader_PS
#)