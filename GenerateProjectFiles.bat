@ECHO OFF

PUSHD %~dp0

SET VSWHERE="%~dp0\Tools\vswhere\vswhere.exe"
SET CMAKE="cmake"
SET VCPKG_ROOT=%~dp0\Tools\vcpkg

REM Check if vswhere.exe exists
IF NOT EXIST %VSWHERE% (
    ECHO ERROR: vswhere.exe not found at %VSWHERE%
    ECHO Please ensure the Tools/vswhere directory contains vswhere.exe
    ECHO.
    PAUSE
    GOTO :Exit
)

REM Check if CMake is available
%CMAKE% --version >NUL 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: CMake not found in PATH.
    ECHO Please install CMake and ensure it's available in your PATH.
    ECHO Download from: https://cmake.org/download/
    ECHO.
    PAUSE
    GOTO :Exit
)

REM Detect latest version of Visual Studio.
SET VS_VERSION=
FOR /F "usebackq delims=." %%i IN (`"%VSWHERE%" -latest -prerelease -requires Microsoft.VisualStudio.Workload.NativeGame -property installationVersion`) DO (
    SET VS_VERSION=%%i
)

IF "%VS_VERSION%" == "18" (
    SET CMAKE_PRESET=vs2026
    SET SOLUTION_FILE=LearningDirectX12.slnx
) ELSE IF "%VS_VERSION%" == "17" (
    SET CMAKE_PRESET=vs2022
    SET SOLUTION_FILE=LearningDirectX12.sln
) ELSE (
    ECHO.
    ECHO ***********************************************************************
    ECHO *                                ERROR                                *
    ECHO ***********************************************************************
    ECHO Detected Visual Studio version %VS_VERSION% is not supported.
    ECHO This script requires Visual Studio 2022 ^(version 17^) or 2026 ^(version 18^).
    ECHO Please install a compatible version with the "Game Development with C++"
    ECHO workload before running this script.
    ECHO.
    PAUSE
    GOTO :Exit
)

REM Binary directory is defined in CMakePresets.json as build/${presetName}
SET CMAKE_BINARY_DIR=build\%CMAKE_PRESET%

ECHO CMake Preset: %CMAKE_PRESET%
ECHO CMake Source Directory: %~dp0
ECHO CMake Binary Directory: %CMAKE_BINARY_DIR%
ECHO Solution file: %SOLUTION_FILE%
ECHO.

%CMAKE% --preset %CMAKE_PRESET%

IF %ERRORLEVEL% NEQ 0 (
    ECHO.
    ECHO ERROR: CMake configuration failed.
    ECHO Please check the output above for details.
    ECHO.
    PAUSE
) ELSE (
    ECHO.
    ECHO ***********************************************************************
    ECHO *                             SUCCESS                                 *
    ECHO ***********************************************************************
    ECHO Project files generated successfully!
    ECHO Opening solution: %SOLUTION_FILE%
    ECHO.
    START %CMAKE_BINARY_DIR%\%SOLUTION_FILE%
)

:Exit

POPD
