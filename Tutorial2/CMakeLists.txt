cmake_minimum_required( VERSION 3.10.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/Tutorial2.h
)

set( SRC_FILES
    src/main.cpp
    src/Tutorial2.cpp
)

add_executable( Tutorial2 WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
)

target_include_directories( Tutorial2
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( Tutorial2
    DX12Lib
)
