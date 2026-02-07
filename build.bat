@echo off
setlocal

echo ===================================
echo   Sea Browser Build Script (Windows)
echo ===================================

if not defined VCPKG_ROOT (
    echo [ERROR] VCPKG_ROOT is not set. Please set it to your vcpkg installation.
    exit /b 1
)

if not exist build-win (
    mkdir build-win
)

cd build-win

echo [INFO] Configuring CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    exit /b %ERRORLEVEL%
)

echo [INFO] Building...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed.
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build completed. Executable is in build-win/Release/seabrowser.exe
pause
