cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

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
)
