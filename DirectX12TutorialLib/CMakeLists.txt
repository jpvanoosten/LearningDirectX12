cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/Application.h
    inc/d3dx12.h
    inc/DirectX12Tutorial.h
    inc/DirectX12TutorialDefines.h
    inc/Events.h
    inc/Game.h
    inc/HighResolutionTimer.h
    inc/Helpers.h
    inc/KeyCodes.h
    inc/Object.h
    inc/Window.h
)

set( SRC_FILES 
    src/Application.cpp
    src/Game.cpp
    src/HighResolutionTimer.cpp
    src/Object.cpp
    src/Window.cpp
)

set( PCH_HEADER_FILE 
    inc/DirectX12TutorialPCH.h
)

set( PCH_SRC_FILE 
    src/DirectX12TutorialPCH.cpp
)

add_library( DirectX12TutorialLib 
    ${HEADER_FILES} 
    ${PCH_HEADER_FILE} 
    ${SRC_FILES}
    ${PCH_SRC_FILE}
)

if( BUILD_SHARED_LIBS )
    set_target_properties( DirectX12TutorialLib
        PROPERTIES
            DEFINE_SYMBOL DX12TL_EXPORTS
    )

    target_compile_definitions( DirectX12TutorialLib
        INTERFACE DX12TL_IMPORTS
    )

    # Disable specific warnings when generating a DLL.
    # Disable warnings:
    #   4251: This warning occurs when DLL exporting a class that contains (private) members deriving from a type in the C++ STL (like std::vector or std::string).
    #         Since these (private) class members are never exported directly, it is safe to ignore this warning.
    target_compile_options( DirectX12TutorialLib
        PUBLIC /wd"4251"
    )
endif()

target_include_directories( DirectX12TutorialLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PUBLIC ${CMAKE_SOURCE_DIR}/externals/boost-1.65.1
)

target_compile_definitions( DirectX12TutorialLib 
    PUBLIC $<$<CONFIG:Debug>:_SCL_SECURE_NO_WARNINGS>
)

target_link_libraries( DirectX12TutorialLib
    d3d12.lib
    dxgi.lib
    dxguid.lib
)

# Enable precompiled headers for faster compiliation.
target_compile_options( DirectX12TutorialLib
    PRIVATE "/YuDirectX12TutorialPCH.h"
)

# set_source_files_properties( ${SRC_FILES}
#    PROPERTIES
#        COMPILE_FLAGS "/YuDirectX12TutorialPCH.h"
#)

set_source_files_properties( ${PCH_SRC_FILE}
    PROPERTIES
        COMPILE_FLAGS "/YcDirectX12TutorialPCH.h"
)
