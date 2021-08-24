cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( TARGET_NAME 06-DXR )

set( HEADER_FILES
    inc/Camera.h
    inc/CameraController.h
    inc/EffectPSO.h
    inc/Light.h
    inc/SceneVisitor.h
    inc/Tutorial6.h
)

set( SRC_FILES
    src/Camera.cpp
    src/CameraController.cpp
    src/EffectPSO.cpp
    src/Light.cpp
    src/main.cpp
    src/SceneVisitor.cpp
    src/Tutorial6.cpp
)

set( VERTEX_SHADERS 
    shaders/Basic_VS.hlsl
)

set( PIXEL_SHADERS
    shaders/Decal_PS.hlsl
    shaders/Lighting_PS.hlsl
    shaders/Unlit_PS.hlsl
)

set( RAYTRACE_SHADERS 
    shaders/Hit_RT.hlsl
    shaders/Miss_RT.hlsl
    shaders/RayGen_RT.hlsl
)

set( SHADER_INCLUDES
    shaders/Base_PS.hlsli
    shaders/Common.hlsli
)

set( SHADERS
    ${VERTEX_SHADERS}
    ${PIXEL_SHADERS}
    ${RAYTRACE_SHADERS}
    ${SHADER_INCLUDES}
)

source_group( "Resources\\Shaders" FILES ${SHADERS} )

set_source_files_properties( ${VERTEX_SHADERS} ${PIXEL_SHADERS} ${RAYTRACE_SHADERS}
    PROPERTIES
        VS_SHADER_OBJECT_FILE_NAME "${CMAKE_SOURCE_DIR}/Data/Shaders/${TARGET_NAME}/%(Filename).cso"
        VS_SHADER_MODEL 6.3
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
)

set_source_files_properties( ${VERTEX_SHADERS}
    PROPERTIES 
        VS_SHADER_TYPE Vertex
)

set_source_files_properties( ${PIXEL_SHADERS}
    PROPERTIES 
        VS_SHADER_TYPE Pixel
)

set_source_files_properties( ${RAYTRACE_SHADERS}
    PROPERTIES
        VS_SHADER_TYPE Library
        VS_SHADER_ENTRYPOINT ""
)

add_executable( ${TARGET_NAME} WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
    ${SHADERS}
)

target_include_directories( ${TARGET_NAME}
    PRIVATE inc
)

target_link_libraries( ${TARGET_NAME}
    GameFramework
    DX12Lib
    assimp
    d3dcompiler.lib
)

# Set Local Debugger Settings (Command Arguments and Environment Variables)
set( COMMAND_ARGUMENTS "-wd \"${CMAKE_SOURCE_DIR}\"" )
configure_file( ${TARGET_NAME}.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.vcxproj.user @ONLY )
