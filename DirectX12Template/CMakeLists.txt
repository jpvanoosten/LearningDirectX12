cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/DX12FrameworkViewSource.h
    inc/DX12FrameworkView.h
    inc/d3dx12.h
)

set( SRC_FILES 
    src/DX12FrameworkView.cpp
    src/DX12FrameworkViewSource.cpp
)

set( PCH_HEADER_FILE 
    inc/DirectX12TemplatePCH.h
)

set( PCH_SRC_FILE 
    src/DirectX12TemplatePCH.cpp
)

add_library( DirectX12TemplateLib SHARED 
    ${HEADER_FILES} 
    ${PCH_HEADER_FILE} 
    ${SRC_FILES}
    ${PCH_SRC_FILE}
)

set_property( TARGET DirectX12TemplateLib
    PROPERTY VS_WINRT_COMPONENT TRUE     # Setting this property to true enables the UWP runtime extensions.
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
