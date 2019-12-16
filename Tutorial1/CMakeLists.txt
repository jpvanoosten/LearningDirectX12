cmake_minimum_required( VERSION 3.16.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/d3dx12.h
    inc/Helpers.h
    resource.h
)

set( SRC_FILES
    src/main.cpp
)

set( RESOURCE_FILES
    Resources/Icon/app_icon.ico
)

source_group( "Resource Files\\Icon" FILES Resources/Icon/app_icon.ico )

add_executable( Tutorial1 WIN32
    ${HEADER_FILES} 
    ${SRC_FILES}
    ${RESOURCE_FILES}
	Tutorial1.rc
)

target_include_directories( Tutorial1
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries( Tutorial1
    d3d12.lib
    dxgi.lib
    dxguid.lib
)
