cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( TARGET_NAME 05-Models )

set( HEADER_FILES
    inc/Camera.h
    inc/Light.h
    inc/Material.h
    inc/Tutorial5.h
)

set( SRC_FILES
    src/Camera.cpp
    src/Light.cpp
    src/main.cpp
    src/Material.cpp
    src/Tutorial5.cpp
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
    assimp
)

# Set Local Debugger Settings (Command Arguments and Environment Variables)
set( COMMAND_ARGUMENTS "-wd \"${CMAKE_SOURCE_DIR}\"" )
configure_file( ${TARGET_NAME}.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.vcxproj.user @ONLY )
