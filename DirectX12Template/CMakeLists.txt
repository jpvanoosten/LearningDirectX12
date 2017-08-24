cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/Application.h
    inc/Window.h
)

set( SRC_FILES 
    src/Application.cpp
    src/Window.cpp
)

add_library( DirectX12TemplateLib STATIC ${HEADER_FILES} ${SRC_FILES} )

target_include_directories( DirectX12TemplateLib 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)