cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( SRC_FILES
    src/main.cpp
)

set( SHADER_FILES
    shaders/VertexShader.hlsl
    shaders/PixelShader.hlsl
)

source_group( "Resources\\Shaders" FILES ${SHADER_FILES} )

set_source_files_properties( shaders/VertexShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.1
)

set_source_files_properties( shaders/PixelShader.hlsl PROPERTIES 
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.1
)

add_executable( 02-Cube WIN32
    ${SRC_FILES}
    ${SHADER_FILES}
)

target_compile_definitions( 02-Cube
    PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE # This is required when using the DirectX Math library as of VS 2017 15.8. See 
)

target_include_directories( 02-Cube
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( 02-Cube
    GameFramework
    DX12Lib
)
