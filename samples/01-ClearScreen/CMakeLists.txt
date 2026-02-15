cmake_minimum_required( VERSION 4.0...4.2.0 ) # Latest version of CMake when this file was created.

set( TARGET_NAME 01-ClearScreen )

set( HEADER_FILES
)

set( SRC_FILES
    src/main.cpp
)

add_executable( ${TARGET_NAME} WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
)

target_include_directories( ${TARGET_NAME}
    PRIVATE inc
)

target_link_libraries( ${TARGET_NAME}
    GameFramework
    DX12Lib
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
