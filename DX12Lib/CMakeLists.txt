cmake_minimum_required( VERSION 3.10.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/DX12LibPCH.h
    inc/Application.h
    inc/CommandQueue.h
    inc/Game.h
    inc/Helpers.h
    inc/HighResolutionClock.h
    resource.h
    inc/Window.h
    inc/d3dx12.h
)


set( SOURCE_FILES
    src/DX12LibPCH.cpp
    src/Application.cpp
    src/CommandQueue.cpp
    src/Game.cpp
    src/HighResolutionClock.cpp
    src/Window.cpp
)

set( RESOURCE_FILES 
    DX12Lib.rc
    Resources/Icon/app_icon.ico
)

source_group( "Resources" FILES DX12Lib.rc )
source_group( "Resources\\Icon" FILES Resources/Icon/app_icon.ico )

add_library( DX12Lib STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
)

target_include_directories( DX12Lib
    PUBLIC inc
)

target_link_libraries( DX12Lib 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/DX12Lib.dir/${CMAKE_CFG_INTDIR}/DX12Lib.res # This is the only way I could figure out how to link the compiled resource file.
)
