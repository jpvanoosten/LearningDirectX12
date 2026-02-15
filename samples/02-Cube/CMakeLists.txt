cmake_minimum_required( VERSION 4.0...4.2.0 ) # Latest version of CMake when this file was created.

set( TARGET_NAME 02-Cube )

set( SRC_FILES
    src/main.cpp
)

set( SHADER_FILES
    shaders/VertexShader.hlsl
    shaders/PixelShader.hlsl
)

source_group( "Resources\\Shaders" FILES ${SHADER_FILES} )

set_source_files_properties( ${SHADER_FILES} 
    PROPERTIES
        VS_SHADER_MODEL 6.0
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
        VS_SHADER_FLAGS $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-Qembed_debug>
)


set_source_files_properties( shaders/VertexShader.hlsl 
PROPERTIES
    VS_SHADER_TYPE Vertex
)

set_source_files_properties( shaders/PixelShader.hlsl 
PROPERTIES
    VS_SHADER_TYPE Pixel
)

add_executable( ${TARGET_NAME} WIN32
    ${SRC_FILES}
    ${SHADER_FILES}
)

target_compile_definitions( ${TARGET_NAME}
    PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE # This is required when using the DirectX Math library as of VS 2017 15.8. See 
)

target_include_directories( ${TARGET_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( ${TARGET_NAME}
    GameFramework
    DX12Lib
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
