cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/Application.h
    inc/GameFrameworkPCH.h
)

set( SOURCE_FILES
    src/Application.cpp
    src/GameFrameworkPCH.cpp
)

set( RESOURCE_FILES
    GameFramework.rc
    resource.h
    Resources/Icon/app_icon.ico
)

source_group( "Resources" FILES GameFramework.rc resource.h )
source_group( "Resources\\Icon" FILES Resources/Icon/app_icon.ico )

add_library( GameFramework STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
)

# Enable C++17 compiler features.
target_compile_features( GameFramework
    PUBLIC cxx_std_17
)

target_include_directories( GameFramework
    PUBLIC inc
)

target_link_libraries( GameFramework
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/GameFramework.dir/${CMAKE_CFG_INTDIR}/GameFramework.res # This is the only way I could figure out how to link the compiled resource file.
)

# Enable precompiled header files.
set_source_files_properties( ${SOURCE_FILES} 
    PROPERTIES 
        COMPILE_FLAGS /Yu"GameFrameworkPCH.h"
)

set_source_files_properties( src/GameFrameworkPCH.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"GameFrameworkPCH.h"
)
