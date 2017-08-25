cmake_minimum_required( VERSION 3.9.1 ) # Latest version of CMake when this file was created.

set( CMAKE_SYSTEM_NAME WindowsStore )   # This is required for a UWP project.
set( CMAKE_SYSTEM_VERSION 10.0 )        # DirectX 12 requires Windows 10.

project( LearningDirectX12 )

add_subdirectory( DirectX12Template )

set( VS_STARTUP_PROJECT DirectX12TemplateLib )


