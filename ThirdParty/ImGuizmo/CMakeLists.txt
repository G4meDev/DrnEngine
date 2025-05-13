cmake_minimum_required(VERSION 3.5)

project(ImGuizmo)

add_library(ImGuizmo STATIC
    GraphEditor.h
    GraphEditor.cpp
    ImCurveEdit.h
    ImCurveEdit.cpp
    ImGradient.h
    ImGradient.cpp
    ImGuizmo.h
    ImGuizmo.cpp
    ImSequencer.h
    ImSequencer.cpp
    ImZoomSlider.h
)

target_include_directories(ImGuizmo
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC inc ../ImGui/
)

target_link_libraries( ImGuizmo
	PUBLIC ImGui
)
