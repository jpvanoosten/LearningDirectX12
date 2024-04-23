@ECHO OFF

PUSHD %~dp0

REM Update these lines if the currently installed version of Visual Studio is not 2022.
SET CMAKE="cmake.exe"
SET CMAKE_GENERATOR="Visual Studio 17 2022"
SET CMAKE_BINARY_DIR=build_vs2022

MKDIR %CMAKE_BINARY_DIR% 2>NUL
PUSHD %CMAKE_BINARY_DIR%

%CMAKE% -G %CMAKE_GENERATOR% -Wno-dev "%~dp0"

IF ERRORLEVEL 1 (
    PAUSE
) ELSE (
    START LearningDirectX12.sln
)

POPD
POPD
