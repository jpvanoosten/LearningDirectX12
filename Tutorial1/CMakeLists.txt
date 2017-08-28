cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES

)

set( SRC_FILES
    src/main.cpp
)

set( PCH_HEADER_FILE
    inc/Tutorial1PCH.h
)

set( PCH_SRC_FILE
    src/Tutorial1PCH.cpp
)

add_executable( Tutorial1 WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
    ${PCH_HEADER_FILE}
    ${PCH_SRC_FILE}
)

target_include_directories( Tutorial1
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( Tutorial1
    PRIVATE DirectX12Template
)

# Enable precompiled headers for faster compiliation.
set_source_files_properties( ${SRC_FILES}
    PROPERTIES
        COMPILE_FLAGS /Yu"Tutorial1PCH.h"
)

set_source_files_properties( ${PCH_SRC_FILE}
    PROPERTIES
        COMPILE_FLAGS /Yc"Tutorial1PCH.h"
)
