cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( TARGET_NAME Game )

set( HEADER_FILES
	Source/GameApplication.h
	Source/GameForwardTypes.h

	Source/Player/TestPlayerCharacter.h
)

set( SRC_FILES
	Source/GamePCH.h
	Source/GamePCH.cpp
	Source/main.cpp
	Source/GameApplication.cpp
	Source/GameForwardTypes.cpp

	Source/Player/TestPlayerCharacter.cpp
)

#set( SHADER_FILES
#    Content/Level_1/Prob_2/M_Prop_2.hlsl
#    Content/Level_1/Prob_2/M_Prop_1.hlsl
#)

file(GLOB_RECURSE SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Content/*.hlsl")

source_group( "Content\\Shaders" FILES ${SHADER_FILES} )

add_executable( ${TARGET_NAME} WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
    ${SHADER_FILES}
)

target_compile_definitions( Engine
    PUBLIC NOMINMAX
)

add_compile_definitions(PX_PHYSX_STATIC_LIB)
add_compile_definitions($<$<CONFIG:Debug>:WITH_EDITOR>)
add_compile_definitions($<$<CONFIG:Debug>:WITH_PVD>)
add_compile_definitions($<$<CONFIG:Debug>:WITH_DEBUG>)
add_compile_definitions($<$<CONFIG:RelWithDebInfo>:WITH_DEBUG>)

target_include_directories( ${TARGET_NAME}
    PRIVATE inc
    PRIVATE inc Source/
    PRIVATE inc ../Engine/Content/Materials/
)

target_link_libraries( ${TARGET_NAME}
    Engine
)

set_source_files_properties( ${SRC_FILES} 
    PROPERTIES 
        COMPILE_FLAGS /Yu"GamePCH.h"
)

set_source_files_properties( Source/GamePCH.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"GamePCH.h"
)

# -----------------------------------------------------------------------------------------------

set_source_files_properties( ${SHADER_FILES}
    PROPERTIES
        VS_SHADER_MODEL 6.2
        VS_SETTINGS "ExcludedFromBuild=true"
)

#foreach(SHADER_FILE IN LISTS SHADER_FILES)
#    set_property(SOURCE ${SHADER_FILE} PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Content/Materials/")
#endforeach()

#set_source_files_properties( Content/Shaders/TestShader_VS.hlsl
#    PROPERTIES
#        VS_SHADER_VARIABLE_NAME TestShader_VS
#)