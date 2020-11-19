@ECHO OFF

PUSHD %~dp0

SET CURL="%~dp0\Tools\curl-7.73.0-win64-mingw\bin\curl.exe"
SET z="%~dp0\Tools\7z1900\7za.exe"

IF NOT EXIST Assets.7z (
    ECHO.
    ECHO Downloading Assets.7z from GitHub.
    %CURL% -LJO https://github.com/jpvanoosten/LearningDirectX12/releases/download/v1.1.0/Assets.7z
    ECHO.
    ECHO Finished downloading Assets.7z.
) ELSE (
    ECHO.
    ECHO Assets.7z already downloaded.
)

IF %ERRORLEVEL% EQU 0 (
    REM Unzip the Assets file to the current directory (replacing any existing files).
    ECHO.
    ECHO Extracting assets...
    %z% x Assets.7z -y
)

ECHO.
ECHO Finished downloading and extracting assets.

POPD
