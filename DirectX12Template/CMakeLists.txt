cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/Application.h
    inc/Window.h
)

set( PCH_HEADER_FILE 
    inc/DirectX12TemplatePCH.h
)

set( SRC_FILES 
    src/Application.cpp
    src/Window.cpp
)

set( PCH_SRC_FILE 
    src/DirectX12TemplatePCH.cpp
)

add_library( DirectX12TemplateLib STATIC 
    ${HEADER_FILES} 
    ${PCH_HEADER_FILE} 
    ${SRC_FILES}
    ${PCH_SRC_FILE}
)

target_include_directories( DirectX12TemplateLib 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# Enable precompiled headers for faster compiliation.
set_source_files_properties( ${SRC_FILES}
    PROPERTIES
        COMPILE_FLAGS /Yu"DirectX12TemplatePCH.h"
)

set_source_files_properties( ${PCH_SRC_FILE}
    PROPERTIES
        COMPILE_FLAGS /Yc"DirectX12TemplatePCH.h"
)
