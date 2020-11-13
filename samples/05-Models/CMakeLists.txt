cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( TARGET_NAME 05-Models )

set( HEADER_FILES
    inc/Camera.h
    inc/CameraController.h
    inc/EffectPSO.h
    inc/Light.h
    inc/SceneVisitor.h
    inc/Tutorial5.h
)

set( SRC_FILES
    src/Camera.cpp
    src/CameraController.cpp
    src/EffectPSO.cpp
    src/Light.cpp
    src/main.cpp
    src/SceneVisitor.cpp
    src/Tutorial5.cpp
)

set( VERTEX_SHADERS 
    shaders/Basic_VS.hlsl
)

set( PIXEL_SHADERS
    shaders/Decal_PS.hlsl
    shaders/Lighting_PS.hlsl
    shaders/Unlit_PS.hlsl
)

set( SHADER_INCLUDES
    shaders/Base_PS.hlsli
)

set( SHADERS
    ${VERTEX_SHADERS}
    ${PIXEL_SHADERS}
    ${SHADER_INCLUDES}
)

source_group( "Resources\\Shaders" FILES ${SHADERS} )

set_source_files_properties( ${VERTEX_SHADERS} ${PIXEL_SHADERS}
    PROPERTIES
        VS_SHADER_OBJECT_FILE_NAME "${CMAKE_SOURCE_DIR}/Data/Shaders/05-Models/%(Filename).cso"
        VS_SHADER_MODEL 5.1
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
)

set_source_files_properties( ${VERTEX_SHADERS}
    PROPERTIES 
        VS_SHADER_TYPE Vertex
)

set_source_files_properties( ${PIXEL_SHADERS}
    PROPERTIES 
        VS_SHADER_TYPE Pixel
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
