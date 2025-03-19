cmake_minimum_required( VERSION 3.16.1 )

#option( DX12LIB_BUILD_SAMPLES "Build samples for DX12Lib" ON )

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

project( DrnEngine LANGUAGES CXX )

# Enable to build shared libraries.
option(BUILD_SHARED_LIBS "Create shared libraries." OFF)

# Enable multithreaded builds
if( MSVC )
    add_compile_options(/MP)
endif()

set( ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build Assimp Tools" FORCE )
set( ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Build Assimp Samples" FORCE )
set( ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build Assimp Tests" FORCE )
set( GAINPUT_SAMPLES OFF CACHE BOOL "Build Samples for Gainput" FORCE )
set( GAINPUT_TESTS OFF CACHE BOOL "Build Tests for Gainput" FORCE)

add_subdirectory( Engine )
add_subdirectory( ThirdParty/ImGui )
add_subdirectory( ThirdParty/assimp )
add_subdirectory( ThirdParty/DirectXTex )
add_subdirectory( ThirdParty/gainput )
add_subdirectory( ThirdParty/DX12Lib )
add_subdirectory( Game )

set_target_properties( assimp IrrXML uninstall UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic 
    PROPERTIES
        FOLDER assimp 
)

set_directory_properties( PROPERTIES VS_STARTUP_PROJECT Game)