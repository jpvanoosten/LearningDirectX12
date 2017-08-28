cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option( BUILD_SHARED_LIBS "Enable shared libraries - Dynamic libraries (.DLL) instead of static libraries (.LIB)" ON )

project( LearningDirectX12 LANGUAGES CXX )

add_subdirectory( DirectX12Template )
add_subdirectory( Tutorial1 )

# Set the startup project.
set_directory_properties( PROPERTIES 
    VS_STARTUP_PROJECT Tutorial1
)
