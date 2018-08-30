cmake_minimum_required( VERSION 3.12.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/Application.h
    inc/CommandQueue.h
    inc/Events.h
    inc/Game.h
    inc/Helpers.h
    inc/HighResolutionClock.h
    inc/KeyCodes.h
    inc/Tutorial2.h
    inc/Window.h
)

set( SRC_FILES
    src/Application.cpp
    src/CommandQueue.cpp
    src/Game.cpp
    src/HighResolutionClock.cpp
    src/main.cpp
    src/Tutorial2.cpp
    src/Window.cpp
)

set( SHADER_FILES
    shaders/VertexShader.hlsl
    shaders/PixelShader.hlsl
)

set( RESOURCE_FILES
    Resources/Icon/app_icon.ico
)

source_group( "Resources" FILES Tutorial2.rc resource.h )
source_group( "Resources\\Icon" FILES Resources/Icon/app_icon.ico )
source_group( "Resources\\Shaders" FILES ${SHADER_FILES} )

set_source_files_properties( shaders/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)

set_source_files_properties( shaders/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

add_executable( Tutorial2 WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
    ${SHADER_FILES}
    ${RESOURCE_FILES}
	Tutorial2.rc
)

target_compile_definitions( Tutorial2
    PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE # This is required when using the DirectX Math library as of VS 2017 15.8. See 
)

target_include_directories( Tutorial2
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( Tutorial2
    d3d12.lib
    dxgi.lib
    dxguid.lib
    D3DCompiler.lib
    Shlwapi.lib
)
