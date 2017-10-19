cmake_minimum_required( VERSION 3.8.0 ) # Latest version of CMake when this file was created.

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option( BUILD_SHARED_LIBS "Enable shared libraries - Dynamic libraries (.DLL) instead of static libraries (.LIB)" ON )

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

project( LearningDirectX12 LANGUAGES CXX )

add_subdirectory( Tutorial1 )

# Set the startup project.
set_directory_properties( PROPERTIES 
    VS_STARTUP_PROJECT Tutorial1
)
