cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( TARGET_NAME ImGui )

set( IMGUI_HEADER
    imconfig.h
    imgui.h
	imgui_impl_win32.h
	imgui_impl_dx12.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h

)

set( IMGUI_SOURCE
	imgui_impl_win32.cpp
	imgui_impl_dx12.cpp
	imgui.cpp
	imgui_draw.cpp
	imgui_widgets.cpp
	imgui_tables.cpp
	imgui_demo.cpp
)

add_library( ImGui STATIC
    ${IMGUI_HEADER}
    ${IMGUI_SOURCE}
)

# Enable C++17 compiler features.
target_compile_features( ImGui 
	PUBLIC cxx_std_17
)

target_include_directories( ${TARGET_NAME}
    PRIVATE inc
)

target_link_libraries( ${TARGET_NAME}
)