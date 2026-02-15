cmake_minimum_required( VERSION 4.0...4.2.0 ) # Latest version of CMake when this file was created.

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
        VS_SHADER_MODEL 6.0
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

set_target_properties( ${TARGET_NAME}
    PROPERTIES
        DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

if(TARGET Microsoft::DirectX12-Agility)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMENT "Copy DirectX12 Agility SDK DLL's"
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${TARGET_NAME}>/D3D12
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:${TARGET_NAME}>/D3D12/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:${TARGET_NAME}>/D3D12/
        COMMAND_EXPAND_LISTS
    )
endif() 
