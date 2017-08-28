cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/Application.h
    inc/d3dx12.h
    inc/DirectX12Template.h
    inc/DirectX12TemplateDefines.h
    inc/Game.h
    inc/Helpers.h
    inc/Window.h
)

set( SRC_FILES 
    src/Application.cpp
    src/Game.cpp
    src/Window.cpp
)

set( PCH_HEADER_FILE 
    inc/DirectX12TemplatePCH.h
)

set( PCH_SRC_FILE 
    src/DirectX12TemplatePCH.cpp
)

add_library( DirectX12Template 
    ${HEADER_FILES} 
    ${PCH_HEADER_FILE} 
    ${SRC_FILES}
    ${PCH_SRC_FILE}
)

if( BUILD_SHARED_LIBS )
    set_target_properties( DirectX12Template
        PROPERTIES
            DEFINE_SYMBOL DX12TL_EXPORTS
    )

    target_compile_definitions( DirectX12Template
        INTERFACE DX12TL_IMPORTS
    )

    # Disable specific warnings when generating a DLL.
    # Disable warnings:
    #   4251: This warning occurs when DLL exporting a class that contains (private) members deriving from a type in the C++ STL (like std::vector or std::string).
    #         Since these (private) class members are never exported directly, it is safe to ignore this warning.
    target_compile_options( DirectX12Template
        PRIVATE /wd"4251"
    )
endif()

target_include_directories( DirectX12Template
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( DirectX12Template 
    d3d12.lib
    dxgi.lib
    dxguid.lib
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
