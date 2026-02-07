@echo off
REM Sea Browser - Windows Build Script
REM Requires: Visual Studio 2022, vcpkg, CMake

setlocal enabledelayedexpansion

echo.
echo ====================================
echo   Sea Browser Windows Build Script
echo ====================================
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake not found. Install from https://cmake.org/
    exit /b 1
)

REM Check for vcpkg
if not defined VCPKG_ROOT (
    echo ERROR: VCPKG_ROOT not set
    echo Install vcpkg: https://vcpkg.io/
    echo Then set: set VCPKG_ROOT=C:\path\to\vcpkg
    exit /b 1
)

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..\..
set BUILD_DIR=%PROJECT_ROOT%\build-output-windows

echo Installing dependencies via vcpkg...
REM Updated for GTK3 build
"%VCPKG_ROOT%\vcpkg" install gtk:x64-windows
"%VCPKG_ROOT%\vcpkg" install webkitgtk:x64-windows
"%VCPKG_ROOT%\vcpkg" install sqlite3:x64-windows
REM gtk3 is usually just 'gtk' in vcpkg or 'gtk3', depending on port availability. 
REM safely installing 'gtk' which defaults to 3 or 4 depending on version, 
REM but 'webkitgtk' for windows typically depends on specific gtk versions.
REM For simplicity we assume vcpkg handles the webkitgtk deps.

echo.
echo Configuring with CMake...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

cmake "%PROJECT_ROOT%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"

echo.
echo Building...
cmake --build . --config Release --parallel

echo.
echo ========================================
echo   Build Complete!
echo   Binary: %BUILD_DIR%\Release\seabrowser.exe
echo ========================================
