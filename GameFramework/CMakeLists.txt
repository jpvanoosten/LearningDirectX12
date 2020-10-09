cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/GameFramework/bitmask_operators.hpp
    inc/GameFramework/CThreadSafeQueue.h
    inc/GameFramework/Events.h
    inc/GameFramework/GameFramework.h
    inc/GameFramework/HighResolutionTimer.h
    inc/GameFramework/KeyCodes.h
    inc/GameFramework/ReadDirectoryChanges.h
    inc/GameFramework/Window.h
)

set( SIGNALS_HEADER_FILES
    inc/signals/optional.hpp
    inc/signals/signals.hpp
)

source_group( "Header Files" FILES ${HEADER_FILES} )
source_group( "signals" FILES ${SIGNALS_HEADER_FILES} )

set( SOURCE_FILES
    src/GameFramework.cpp
    src/GameFrameworkPCH.h
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
    ${SIGNALS_HEADER_FILES}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
    ../.clang-format
)

# Enable C++17 compiler features.
target_compile_features( GameFramework
    PUBLIC cxx_std_17
)

# Remove those pesky min/max macros defined in global scope.
target_compile_definitions( GameFramework
    PUBLIC NOMINMAX 
)

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
