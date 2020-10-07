cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/GameFramework.h
    inc/bitmask_operators.hpp
    inc/CThreadSafeQueue.h
    inc/Events.h
    inc/GameFrameworkPCH.h
    inc/HighResolutionTimer.h
    inc/KeyCodes.h
    inc/ReadDirectoryChanges.h
    inc/Window.h
    inc/optional.hpp
    inc/signals.hpp
)

source_group( "Header Files" FILES ${HEADER_FILES} )
source_group( "signals" FILES inc/optional.hpp inc/signals.hpp )

set( SOURCE_FILES
    src/GameFramework.cpp
    src/GameFrameworkPCH.cpp
    src/HighResolutionTimer.cpp
    src/ReadDirectoryChanges.cpp
    src/ReadDirectoryChangesPrivate.h
    src/ReadDirectoryChangesPrivate.cpp
    src/Window.cpp
)

source_group( "Source Files" FILES ${SOURCE_FILES} )

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
    ../.clang-format
)

# Enable C++17 compiler features.
target_compile_features( GameFramework
    PUBLIC cxx_std_17
)

# Remove those pesky min/max macros defined in global scope.
target_compile_definitions( GameFramework PUBLIC NOMINMAX )

target_include_directories( GameFramework
    PUBLIC inc
)

target_link_libraries( GameFramework
    PUBLIC gainput
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
