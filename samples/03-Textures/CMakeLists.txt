cmake_minimum_required( VERSION 3.16.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/Camera.h
    inc/Light.h
    inc/SceneVisitor.h
    inc/Tutorial3.h
)

set( SRC_FILES
    src/Camera.cpp
    src/main.cpp
    src/Light.cpp
    src/SceneVisitor.cpp
    src/Tutorial3.cpp
)

set( SHADER_FILES
    shaders/VertexShader.hlsl
    shaders/UnlitPixelShader.hlsl
    shaders/PixelShader.hlsl
)

source_group( "Resources\\Shaders" FILES ${SHADER_FILES} )

set_source_files_properties( ${SHADER_FILES} 
    PROPERTIES
        VS_SHADER_OBJECT_FILE_NAME "${CMAKE_SOURCE_DIR}/Data/Shaders/03-Textures/%(Filename).cso"
        VS_SHADER_MODEL 6.0
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
)

set_source_files_properties( shaders/VertexShader.hlsl 
    PROPERTIES 
        VS_SHADER_TYPE Vertex
)

set_source_files_properties( shaders/PixelShader.hlsl shaders/UnlitPixelShader.hlsl
    PROPERTIES 
        VS_SHADER_TYPE Pixel
)

add_executable( 03-Textures WIN32
    ${HEADER_FILES}
    ${SRC_FILES}
    ${SHADER_FILES}
)

target_include_directories( 03-Textures
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( 03-Textures
    DX12Lib
    GameFramework
    d3dcompiler.lib
    Shlwapi.lib
)

# Set Local Debugger Settings (Command Arguments and Environment Variables)
set( COMMAND_ARGUMENTS "-wd \"${CMAKE_SOURCE_DIR}\"" )
configure_file( 03-Textures.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/03-Textures.vcxproj.user @ONLY )
