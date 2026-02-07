# Sea Browser - Windows Build Script (PowerShell)
# Requires: Visual Studio 2022, vcpkg, CMake

param(
    [switch]$Clean,
    [switch]$Install,
    [string]$BuildType = "Release"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  Sea Browser Windows Build Script" -ForegroundColor Cyan  
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$BuildDir = Join-Path $ProjectRoot "build-output-windows"

# Check for CMake
try {
    $null = Get-Command cmake -ErrorAction Stop
    Write-Host "[OK] CMake found" -ForegroundColor Green
} catch {
    Write-Host "[ERROR] CMake not found. Install from https://cmake.org/" -ForegroundColor Red
    exit 1
}

# Check for vcpkg
if (-not $env:VCPKG_ROOT) {
    Write-Host "[ERROR] VCPKG_ROOT environment variable not set" -ForegroundColor Red
    Write-Host ""
    Write-Host "Install vcpkg:" -ForegroundColor Yellow
    Write-Host "  git clone https://github.com/microsoft/vcpkg.git C:\vcpkg"
    Write-Host "  C:\vcpkg\bootstrap-vcpkg.bat"
    Write-Host "  [Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\vcpkg', 'User')"
    exit 1
}
Write-Host "[OK] vcpkg found at $env:VCPKG_ROOT" -ForegroundColor Green

# Install dependencies
Write-Host ""
Write-Host "Installing dependencies via vcpkg..." -ForegroundColor Yellow

$packages = @(
    "gtk:x64-windows",
    "webkitgtk:x64-windows",
    "sqlite3:x64-windows"
)

foreach ($pkg in $packages) {
    Write-Host "  Installing $pkg..."
    & "$env:VCPKG_ROOT\vcpkg" install $pkg --quiet
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[WARNING] Could not install $pkg - may already be installed or unavailable" -ForegroundColor Yellow
    }
}

# Clean if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host ""
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure
Write-Host ""
Write-Host "Configuring with CMake..." -ForegroundColor Yellow
Push-Location $BuildDir

cmake $ProjectRoot `
    -DCMAKE_BUILD_TYPE=$BuildType `
    -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] CMake configuration failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host ""
Write-Host "Building..." -ForegroundColor Yellow
cmake --build . --config $BuildType --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Build failed" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Success
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  Build Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Binary: $BuildDir\$BuildType\seabrowser.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run:" -ForegroundColor Yellow
Write-Host "  & '$BuildDir\$BuildType\seabrowser.exe'"
Write-Host ""

if ($Install) {
    Write-Host "Installing..." -ForegroundColor Yellow
    $InstallDir = "$env:LOCALAPPDATA\SeaBrowser"
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Copy-Item "$BuildDir\$BuildType\seabrowser.exe" $InstallDir
    Write-Host "Installed to: $InstallDir" -ForegroundColor Green
}
