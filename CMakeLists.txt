cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( CMAKE_SYSTEM_NAME WindowsStore )   # This is required for a UWP project.
set( CMAKE_SYSTEM_VERSION 10.0 )        # DirectX 12 requires Windows 10.

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project( LearningDirectX12 LANGUAGES CXX )

add_subdirectory( DirectX12Template )
add_subdirectory( Tutorial1 )

# Set the startup project.
set_directory_properties( PROPERTIES 
    VS_STARTUP_PROJECT Tutorial1
)
