cmake_minimum_required( VERSION 3.16.1 ) # Latest version of CMake when this file was created.

option( DX12LIB_BUILD_SAMPLES "Build samples for DX12Lib" ON )

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

project( LearningDirectX12 LANGUAGES CXX )

# Enable to build shared libraries.
option(BUILD_SHARED_LIBS "Create shared libraries." OFF)

# Enable multithreaded builds
if( MSVC )
    add_compile_options(/MP)
endif()

set( ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build Assimp Tools" FORCE )
set( ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Build Assimp Samples" FORCE )
set( ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build Assimp Tests" FORCE )
set( GAINPUT_SAMPLES OFF CACHE BOOL "Build Samples for Gainput" FORCE )
set( GAINPUT_TESTS OFF CACHE BOOL "Build Tests for Gainput" FORCE)

add_subdirectory( extern/assimp )

set_target_properties( assimp IrrXML uninstall UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic 
    PROPERTIES
        FOLDER assimp 
)

add_subdirectory( extern/gainput )
add_subdirectory( extern/DirectXTex )
add_subdirectory( GameFramework )
add_subdirectory( DX12Lib )

if ( DX12LIB_BUILD_SAMPLES )
    add_subdirectory( Samples/01-ClearScreen)
    add_subdirectory( Samples/02-Cube )
    add_subdirectory( Samples/03-Textures )
    add_subdirectory( Samples/04-HDR )
    add_subdirectory( Samples/05-Models )

    set_target_properties( 01-ClearScreen 02-Cube 03-Textures 04-HDR 05-Models
        PROPERTIES
            FOLDER Samples
    )

    # Set the startup project.
    set_directory_properties( PROPERTIES 
        VS_STARTUP_PROJECT 05-Models
    )
endif( DX12LIB_BUILD_SAMPLES )