cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( TARGET_NAME Game )

set( HEADER_FILES
	Source/GameApplication.h
)

set( SRC_FILES
	Source/main.cpp
	Source/GameApplication.cpp
)

add_executable( ${TARGET_NAME} WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
)

target_include_directories( ${TARGET_NAME}
    PRIVATE inc
)

target_link_libraries( ${TARGET_NAME}
    Engine
)

#set( COMMAND_ARGUMENTS "-wd \"${CMAKE_SOURCE_DIR}\"" )
#configure_file( 01-ClearScreen.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/01-ClearScreen.vcxproj.user @ONLY )
