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

add_executable( ${TARGET_NAME} WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
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

#set( COMMAND_ARGUMENTS "-wd \"${CMAKE_SOURCE_DIR}\"" )
#configure_file( 01-ClearScreen.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/01-ClearScreen.vcxproj.user @ONLY )
